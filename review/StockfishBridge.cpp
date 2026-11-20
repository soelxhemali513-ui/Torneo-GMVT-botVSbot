// StockfishBridge.cpp
// IMPORTANTE: questo file include windows.h ma MAI raylib.h.
// Questo è l'unico punto del progetto dove windows.h entra,
// quindi non ci sono conflitti con raylib.
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>

#include "StockfishBridge.h"
#include <algorithm>
#include <thread>
#include <chrono>

// Cast helper: void* <-> HANDLE
static inline HANDLE H(void* p) { return reinterpret_cast<HANDLE>(p); }

StockfishBridge::StockfishBridge() {}

StockfishBridge::~StockfishBridge() { shutdown(); }

bool StockfishBridge::launch(const std::string& path) {
    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;

    HANDLE hStdinR, hStdoutW;
    HANDLE hStdinW, hStdoutR;
    if (!CreatePipe(&hStdinR,  &hStdinW,  &sa, 0)) return false;
    if (!CreatePipe(&hStdoutR, &hStdoutW, &sa, 0)) return false;

    SetHandleInformation(hStdinW,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hStdoutR, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = {};
    si.cb          = sizeof(STARTUPINFOA);
    si.hStdInput   = hStdinR;
    si.hStdOutput  = hStdoutW;
    si.hStdError   = hStdoutW;
    si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {};
    std::string cmd = path;
    bool ok = CreateProcessA(NULL, cmd.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

    CloseHandle(hStdinR);
    CloseHandle(hStdoutW);

    if (!ok) return false;

    hStdinWrite = reinterpret_cast<void*>(hStdinW);
    hStdoutRead = reinterpret_cast<void*>(hStdoutR);
    hProcess    = reinterpret_cast<void*>(pi.hProcess);
    hThread     = reinterpret_cast<void*>(pi.hThread);
    alive       = true;

    send("uci");
    readUntil("uciok", 3000);
    send("isready");
    readUntil("readyok", 3000);
    send("ucinewgame");
    return true;
}

void StockfishBridge::shutdown() {
    if (!alive) return;
    send("quit");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CloseHandle(H(hStdinWrite));
    CloseHandle(H(hStdoutRead));
    CloseHandle(H(hProcess));
    CloseHandle(H(hThread));
    hStdinWrite = hStdoutRead = hProcess = hThread = nullptr;
    alive = false;
}

void StockfishBridge::send(const std::string& cmd) {
    std::string line = cmd + "\n";
    DWORD written;
    WriteFile(H(hStdinWrite), line.c_str(), (DWORD)line.size(), &written, NULL);
}

std::string StockfishBridge::readUntil(const std::string& keyword, int timeoutMs) {
    std::string result;
    char buf[512];
    DWORD bytesRead;
    auto start = std::chrono::steady_clock::now();

    while (true) {
        DWORD available = 0;
        PeekNamedPipe(H(hStdoutRead), NULL, 0, NULL, &available, NULL);
        if (available > 0) {
            DWORD toRead = std::min(available, (DWORD)(sizeof(buf) - 1));
            ReadFile(H(hStdoutRead), buf, toRead, &bytesRead, NULL);
            buf[bytesRead] = '\0';
            result += buf;
            if (result.find(keyword) != std::string::npos) break;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > timeoutMs)
            break;
    }
    return result;
}

Evaluation StockfishBridge::getEval(const std::string& fen, int depth) {
    send("position fen " + fen);
    send("go depth " + std::to_string(depth));
    return parseEval(readUntil("bestmove", 5000), fen);
}

StockfishBridge::EvalResult StockfishBridge::getEvalWithMove(const std::string& fen, int depth) {
    send("position fen " + fen);
    send("go depth " + std::to_string(depth));
    std::string output = readUntil("bestmove", 5000);

    EvalResult r;
    r.eval = parseEval(output, fen);

    size_t bm = output.find("bestmove ");
    if (bm != std::string::npos) {
        size_t start = bm + 9;
        size_t end   = output.find_first_of(" \n\r", start);
        r.topMove    = output.substr(start, end - start);
    }
    return r;
}

Evaluation StockfishBridge::parseEval(const std::string& output, const std::string& fen) {
    bool blackToMove = (fen.find(" b ") != std::string::npos);
    Evaluation result { Evaluation::Type::CENTIPAWN, 0 };

    size_t pos = 0, found;
    while ((found = output.find("score cp ", pos)) != std::string::npos) {
        try {
            int val = std::stoi(output.substr(found + 9));
            result.type  = Evaluation::Type::CENTIPAWN;
            result.value = blackToMove ? -val : val;
        } catch (...) {}
        pos = found + 1;
    }
    pos = 0;
    while ((found = output.find("score mate ", pos)) != std::string::npos) {
        try {
            int mate = std::stoi(output.substr(found + 11));
            result.type  = Evaluation::Type::MATE;
            result.value = blackToMove ? -mate : mate;
        } catch (...) {}
        pos = found + 1;
    }
    return result;
}

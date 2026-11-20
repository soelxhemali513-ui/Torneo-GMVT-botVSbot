#pragma once
// StockfishBridge.h — NON include windows.h, usa void* per i HANDLE opachi.
// Tutta la roba Win32 sta in StockfishBridge.cpp che non include raylib.h.
#include "Classify.h"
#include <string>

class StockfishBridge {
public:
    StockfishBridge();
    ~StockfishBridge();

    bool launch(const std::string& path);

    struct EvalResult {
        Evaluation eval;
        std::string topMove;
    };

    Evaluation   getEval(const std::string& fen, int depth = 14);
    EvalResult   getEvalWithMove(const std::string& fen, int depth = 14);
    bool         isAlive() const { return alive; }
    void         shutdown();

private:
    void        send(const std::string& cmd);
    std::string readUntil(const std::string& keyword, int timeoutMs);
    Evaluation  parseEval(const std::string& output, const std::string& fen);

    // HANDLE come void* — i cast avvengono solo in StockfishBridge.cpp
    void* hStdinWrite  = nullptr;
    void* hStdoutRead  = nullptr;
    void* hProcess     = nullptr;
    void* hThread      = nullptr;
    bool  alive        = false;
};

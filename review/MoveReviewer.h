#pragma once
#include "StockfishBridge.h"
#include "Classify.h"
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>

// ============================================================
// Struttura che rappresenta una mossa classificata
// ============================================================
struct ReviewedMove {
    std::string moveUCI;        // es. "e2e4"
    std::string moveSAN;        // es. "e4" (opzionale, se lo passi tu)
    bool        isWhite;

    Evaluation  evalBefore;     // Stockfish PRIMA della mossa
    Evaluation  evalAfter;      // Stockfish DOPO la mossa
    std::string topMoveBefore;  // mossa migliore nella pos. prima

    double      pointLoss;      // perdita di Win% [0..1]
    double      accuracy;       // accuratezza [0..100]
    Classification classification;

    int         moveNumber;     // es. 1, 2, 3...
};

// ============================================================
// MoveReviewer - collega Stockfish al loop di gioco
//
// Uso:
//   reviewer.init("stockfish.exe", 14);
//
//   // Prima che il bot giochi:
//   reviewer.onBeforeMove(fenCorrente, mosse_legali_count);
//
//   // Dopo che il bot ha giocato:
//   reviewer.onAfterMove("e2e4", nuovoFen, isCheckmate);
//
//   // Nel render:
//   reviewer.getMoves() → vector<ReviewedMove>
// ============================================================

class MoveReviewer {
public:
    MoveReviewer() = default;

    // -------------------------------------------------------
    // Inizializza e lancia Stockfish.
    // depth: profondità analisi (14 = veloce, 18 = buona)
    // -------------------------------------------------------
    bool init(const std::string& stockfishPath, int depth = 14) {
        analysisDepth = depth;
        return sf.launch(stockfishPath);
    }

    // -------------------------------------------------------
    // Chiama PRIMA che il bot giochi la sua mossa.
    // fen: FEN della posizione attuale
    // legalMoveCount: numero di mosse legali disponibili
    // -------------------------------------------------------
    void onBeforeMove(const std::string& fen, int legalMoveCount = 99) {
        pendingFen = fen;
        pendingLegalMoveCount = legalMoveCount;

        // Valuta la posizione attuale + prendi la top move
        auto result = sf.getEvalWithMove(fen, analysisDepth);
        pendingEvalBefore = result.eval;
        pendingTopMove    = result.topMove;
        pendingWhite      = (fen.find(" w ") != std::string::npos);
    }

    // -------------------------------------------------------
    // Chiama DOPO che il bot ha giocato.
    // moveUCI: mossa giocata in UCI (es. "e2e4")
    // newFen:  FEN dopo la mossa
    // isCheckmate: la mossa ha dato scaccomatto?
    // -------------------------------------------------------
    void onAfterMove(
        const std::string& moveUCI,
        const std::string& newFen,
        const std::string& moveSAN = "",
        bool isCheckmate = false
    ) {
        Evaluation evalAfter = sf.getEval(newFen, analysisDepth);
        currentEval = evalAfter;

        bool isTopMove = (!pendingTopMove.empty() &&
                          moveUCI.substr(0,4) == pendingTopMove.substr(0,4));
        bool isForced  = (pendingLegalMoveCount <= 1);

        double loss = getExpectedPointsLoss(pendingEvalBefore, evalAfter, pendingWhite);
        double acc  = getMoveAccuracy(loss);

        ReviewedMove rm;
        rm.moveUCI        = moveUCI;
        rm.moveSAN        = moveSAN.empty() ? moveUCI : moveSAN;
        rm.isWhite        = pendingWhite;
        rm.evalBefore     = pendingEvalBefore;
        rm.evalAfter      = evalAfter;
        rm.topMoveBefore  = pendingTopMove;
        rm.pointLoss      = loss;
        rm.accuracy       = acc;
        rm.moveNumber     = (int)(moves.size() / 2) + 1;
        rm.classification = classify(
            pendingEvalBefore, evalAfter,
            pendingWhite, isTopMove, isForced, isCheckmate
        );

        {
            std::lock_guard<std::mutex> lock(movesMutex);
            moves.push_back(rm);
        }

        if (onMoveClassified) {
            onMoveClassified(rm);
        }
    }

    // -------------------------------------------------------
    // Reset per nuova partita
    // -------------------------------------------------------
    void reset() {
        std::lock_guard<std::mutex> lock(movesMutex);
        moves.clear();
        currentEval = { Evaluation::Type::CENTIPAWN, 0 };
    }

    // -------------------------------------------------------
    // Accessori
    // -------------------------------------------------------
    std::vector<ReviewedMove> getMoves() const {
        std::lock_guard<std::mutex> lock(movesMutex);
        return moves;
    }

    Evaluation getCurrentEval() const { return currentEval; }

    bool isReady() const { return sf.isAlive(); }

    // Callback opzionale: chiamata ogni volta che una mossa viene classificata
    std::function<void(const ReviewedMove&)> onMoveClassified;

private:
    StockfishBridge sf;
    int analysisDepth = 14;

    // Dati della posizione corrente (prima della mossa)
    std::string  pendingFen;
    Evaluation   pendingEvalBefore = { Evaluation::Type::CENTIPAWN, 0 };
    std::string  pendingTopMove;
    bool         pendingWhite = true;
    int          pendingLegalMoveCount = 99;

    // Risultati
    mutable std::mutex movesMutex;
    std::vector<ReviewedMove> moves;
    Evaluation currentEval = { Evaluation::Type::CENTIPAWN, 0 };
};

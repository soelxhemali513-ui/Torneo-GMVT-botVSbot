#pragma once
#include "Classification.h"
#include <cmath>
#include <algorithm>

// ============================================================
// Port di shared/src/lib/reporter/expectedPoints.ts
//           shared/src/lib/reporter/classification/pointLoss.ts
//
// Il sito NON usa centipawn grezzi per classificare.
// Usa la Win% (Expected Points) con funzione sigmoide,
// esattamente come chess.com.
// ============================================================

struct Evaluation {
    enum class Type { CENTIPAWN, MATE } type;
    int value; // centipawn dal punto di vista del BIANCO, oppure mosse al matto (positivo = vantaggio)
};

// -------------------------------------------------------
// getExpectedPoints (expectedPoints.ts)
// Restituisce la probabilità di vittoria per il BIANCO [0..1]
// usando la sigmoide con gradiente 0.0035 (identico al sito)
// -------------------------------------------------------
inline double getExpectedPoints(const Evaluation& eval) {
    if (eval.type == Evaluation::Type::MATE) {
        if (eval.value == 0) return 0.5;
        return eval.value > 0 ? 1.0 : 0.0;
    }
    // sigmoid(0.0035 * cp)
    return 1.0 / (1.0 + std::exp(-0.0035 * eval.value));
}

// -------------------------------------------------------
// getExpectedPointsLoss (expectedPoints.ts)
// Perdita di Win% per il giocatore che ha mosso
// -------------------------------------------------------
inline double getExpectedPointsLoss(
    const Evaluation& evalBefore,
    const Evaluation& evalAfter,
    bool isWhite
) {
    double wpBefore = getExpectedPoints(evalBefore); // Win% bianco prima
    double wpAfter  = getExpectedPoints(evalAfter);  // Win% bianco dopo

    // Per il bianco: vuole wp alta → perdita = wpBefore - wpAfter
    // Per il nero:   vuole wp bassa → perdita = wpAfter - wpBefore (flipped)
    double loss = isWhite
        ? (wpBefore - wpAfter)
        : (wpAfter  - wpBefore);

    return std::max(0.0, loss);
}

// -------------------------------------------------------
// getMoveAccuracy (accuracy.ts)
// Formula: 103.16 * exp(-4 * pointLoss) - 3.17
// -------------------------------------------------------
inline double getMoveAccuracy(double pointLoss) {
    double acc = 103.16 * std::exp(-4.0 * pointLoss) - 3.17;
    return std::max(0.0, std::min(100.0, acc));
}

// -------------------------------------------------------
// pointLossClassify (classification/pointLoss.ts)
// Gestisce tutti i 4 casi: cp→cp, mate→mate, cp→mate, mate→cp
// -------------------------------------------------------
inline Classification pointLossClassify(
    const Evaluation& evalBefore,
    const Evaluation& evalAfter,
    bool isWhite
) {
    // Valore soggettivo (dal punto di vista del giocatore che ha mosso)
    auto subjective = [&](const Evaluation& e) -> int {
        return isWhite ? e.value : -e.value;
    };

    int prevSubj = subjective(evalBefore);
    int currSubj = subjective(evalAfter);

    // --- Caso 1: mate → mate ---
    if (evalBefore.type == Evaluation::Type::MATE &&
        evalAfter.type  == Evaluation::Type::MATE) {

        // Da matto vincente a matto perdente
        if (prevSubj > 0 && currSubj < 0) {
            return currSubj < -3
                ? Classification::MISTAKE
                : Classification::BLUNDER;
        }

        // mateLoss: valore negativo = matto più vicino (buono per chi vince)
        // Per il perdente, mantenere lo stesso numero di mosse al matto è BEST
        int mateLoss = (evalAfter.value - evalBefore.value) * (isWhite ? 1 : -1);

        if (mateLoss < 0 || (mateLoss == 0 && currSubj < 0))
            return Classification::BEST;
        else if (mateLoss < 2)
            return Classification::EXCELLENT;
        else if (mateLoss < 7)
            return Classification::OKAY;
        else
            return Classification::INACCURACY;
    }

    // --- Caso 2: mate → centipawn (ho perso il matto) ---
    if (evalBefore.type == Evaluation::Type::MATE &&
        evalAfter.type  == Evaluation::Type::CENTIPAWN) {

        if (currSubj >= 800)  return Classification::EXCELLENT;
        if (currSubj >= 400)  return Classification::OKAY;
        if (currSubj >= 200)  return Classification::INACCURACY;
        if (currSubj >= 0)    return Classification::MISTAKE;
        return Classification::BLUNDER;
    }

    // --- Caso 3: centipawn → mate (ho trovato/subito matto) ---
    if (evalBefore.type == Evaluation::Type::CENTIPAWN &&
        evalAfter.type  == Evaluation::Type::MATE) {

        if (currSubj > 0)   return Classification::BEST;       // trovo matto
        if (currSubj >= -2) return Classification::BLUNDER;
        if (currSubj >= -5) return Classification::MISTAKE;
        return Classification::INACCURACY;
    }

    // --- Caso 4: centipawn → centipawn (caso più comune) ---
    double loss = getExpectedPointsLoss(evalBefore, evalAfter, isWhite);

    if (loss < 0.01)  return Classification::BEST;
    if (loss < 0.045) return Classification::EXCELLENT;
    if (loss < 0.08)  return Classification::OKAY;
    if (loss < 0.12)  return Classification::INACCURACY;
    if (loss < 0.22)  return Classification::MISTAKE;
    return Classification::BLUNDER;
}

// -------------------------------------------------------
// classify() principale - chiamata da MoveReviewer
// include forzato (una sola mossa legale) e mossa top
// -------------------------------------------------------
inline Classification classify(
    const Evaluation& evalBefore,   // eval PRIMA della mossa (top line Stockfish)
    const Evaluation& evalAfter,    // eval DOPO la mossa (top line Stockfish)
    bool isWhite,
    bool isTopMove,                 // la mossa giocata era la top di Stockfish?
    bool isOnlyLegalMove,           // una sola mossa legale disponibile?
    bool isCheckmate                // la mossa ha dato scaccomatto?
) {
    // Mossa forzata (una sola legale)
    if (isOnlyLegalMove)
        return Classification::FORCED;

    // Matto dato → sempre BEST
    if (isCheckmate)
        return Classification::BEST;

    // Era la mossa migliore?
    if (isTopMove)
        return Classification::BEST;

    // Altrimenti classifica per perdita di Win%
    return pointLossClassify(evalBefore, evalAfter, isWhite);
}

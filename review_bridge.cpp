#include "review_bridge.h"
#include "variabili.h"
#include "pezzi.h"

MoveReviewer g_reviewer;
ReviewPanel  g_reviewPanel;

void review_init() {
    // Stockfish nella stessa cartella dell'exe
    if (!g_reviewer.init("stockfish-windows-x86-64-avx2.exe", 14))
        TraceLog(LOG_WARNING, "REVIEW: Stockfish non trovato!");

    // Icone PNG dal sito — copia client/public/img/classifications/ accanto all'exe
    g_reviewPanel.loadIcons("img/classifications/");
}

void review_pre_move() {
    if (!g_reviewer.isReady()) return;
    int legal_count = 0;
    // conta quante mosse legali ha il turno corrente (approssimazione: usa mosse_possibili
    // che viene già calcolato da autoMove prima di valuta_mossa)
    legal_count = mosse_possibili > 0 ? mosse_possibili : 99;
    g_reviewer.onBeforeMove(board_to_fen(), legal_count);
}

void review_post_move(int from, int to, bool is_matto) {
    if (!g_reviewer.isReady()) return;
    std::string uci = move_to_uci(from, to, promozione ? b.casa[to] : 0);
    g_reviewer.onAfterMove(uci, board_to_fen(), uci, is_matto);
}

void review_reset() {
    g_reviewer.reset();
}

void review_draw(int x, int y, int w, int h) {
    Rectangle r = { (float)x, (float)y, (float)w, (float)h };
    g_reviewPanel.draw(r, g_reviewer);
}

const ReviewedMove* review_last_move() {
    auto moves = g_reviewer.getMoves();
    if (moves.empty()) return nullptr;
    static ReviewedMove last;
    last = moves.back();
    return &last;
}

// Ritorna l'ultima mossa classificata (nullptr se non ancora disponibile)

void review_draw_badge(int col, int row, int ox, int oy, int sq) {
    const ReviewedMove* lm = review_last_move();
    if (!lm) return;
    g_reviewPanel.drawBadge(lm->classification, col, row, ox, oy, sq);
}

#pragma once
// review_bridge.h — unico punto di contatto tra il tuo codice e il sistema review
// Include questo in main.cpp e autoMoveBot.cpp, nient'altro va toccato.

#include "fen_utils.h"
#include "review/MoveReviewer.h"
#include "review/ReviewPanel.h"

// Istanze globali (definite in review_bridge.cpp)
extern MoveReviewer g_reviewer;
extern ReviewPanel  g_reviewPanel;

// Chiama in main() dopo InitWindow()
void review_init();

// Chiama PRIMA che valuta_mossa() venga eseguita
void review_pre_move();

// Chiama DOPO esegui_mossa(), passando from/to e se è stato matto
void review_post_move(int from, int to, bool is_matto);

// Chiama in prepara_game() (reset per nuova partita)
void review_reset();

// Chiama nel loop di render
void review_draw(int x, int y, int w, int h);

// Ritorna l'ultima mossa classificata (nullptr se non ancora disponibile)
const ReviewedMove* review_last_move();

void review_draw_badge(int col, int row, int ox, int oy, int sq);

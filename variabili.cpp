#include "scacchiera.h"
#include "variabili.h"
#include <cstdint>
#include <vector>
#include "raylib.h"

Sound suono_move;
Sound suono_promozione;
Sound suono_capture;
Sound suono_scacco;
Sound suono_arrocco;

bool selezione    = false;
bool mossa        = true;
bool check        = false;
bool aggiorna_mosse = false;
bool matto        = false;
bool enpassant    = false;
bool promozione   = false;
bool mossa_fatta  = true;
bool re_e1        = true;
bool re_e8        = true;
bool torre_a1     = true;
bool torre_a8     = true;
bool torre_h1     = true;
bool torre_h8     = true;
bool bot_bianco   = false;
bool show_stats   = false;

int col, riga, idx, pezzo, mosse_possibili = 0, idx2, turno;
int idx_enpassant = -1, idx_promozione = -1;
int righe = 8, colonne = 8;

// larghezza_case e altezza_case rimangono const per compatibilità
// con tutto il codice di scacchiera.cpp che li usa per logica interna
const int larghezza_case = 100;
const int altezza_case   = 100;

int mossa_possibile[27];
int casa_re = 60, avversario, prom = 0;
scacchiera b;
bool menu        = true;
std::string stringa_fen;
int fifty_move   = 0;
bool draw        = false;
bool motore_in_ricerca = false;
long long nodes_searched = 0;
int nodes_per_second = 0, profondita_raggiunta = 0, tempo_ultima_mossa_ms = 0;
std::vector<uint64_t> history;
std::vector<StatoMossa> storia_mosse;
int mossaC = -1, mossaR = -1, mossaC_a = -1, mossaR_a = -1;
bool si_evidenzia = false;
int evidenzia = -1, quante_evid = 0;
int evidenziate[64];

// ── Layout dinamico ──────────────────────────────────────────────────────────
int board_offset_x = 0;
int board_offset_y = 0;
int sq_size        = 100;
int panel_x_dyn    = 820;
int panel_w_dyn    = 360;

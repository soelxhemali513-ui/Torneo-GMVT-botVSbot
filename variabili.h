#pragma once
#include "scacchiera.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include "raylib.h"
extern std::vector<uint64_t> history;

class StatoMossa {
public:
    int idx;
    int idx2;
    int pezzo;
    int catturato;
    int idx_enpassant_prec;
    int pedone_ep_idx;
    int pedone_ep_valore;
    int torre_da;
    int torre_a;
    int torre_valore;
    bool re_e1_prec, re_e8_prec;
    bool torre_a1_prec, torre_a8_prec, torre_h1_prec, torre_h8_prec;
    int fifty_move_prec;
    int turno_prec;
    bool era_promozione;
};

extern std::vector<StatoMossa> storia_mosse;
extern bool selezione;
extern bool mossa;
extern bool check;
extern bool aggiorna_mosse;
extern bool matto;
extern bool enpassant;
extern bool promozione;
extern bool mossa_fatta;
extern bool bot_bianco;
extern bool torre_a1;
extern bool torre_a8;
extern bool torre_h1;
extern bool torre_h8;
extern bool re_e1;
extern bool re_e8;
extern int col;
extern int riga;
extern int idx;
extern int pezzo;
extern int mosse_possibili;
extern int idx2;
extern int turno;
extern int idx_enpassant;
extern int idx_promozione;
extern int righe;
extern int colonne;
extern const int larghezza_case;   // mantenuto per compatibilità con scacchiera.cpp
extern const int altezza_case;
extern int mossa_possibile[27];
extern int casa_re;
extern int avversario;
extern int prom;
extern scacchiera b;
extern std::string stringa_fen;
extern bool menu;
extern int fifty_move;
extern bool draw;
extern bool show_stats;
extern bool motore_in_ricerca;
extern long long nodes_searched;
extern int nodes_per_second;
extern int profondita_raggiunta;
extern int tempo_ultima_mossa_ms;
extern int mossaC;
extern int mossaR;
extern int mossaR_a;
extern int mossaC_a;
extern bool si_evidenzia;
extern int evidenzia;
extern int evidenziate[];
extern int quante_evid;

// ── Layout dinamico (torneo fullscreen) ──────────────────────────────────────
extern int board_offset_x;   // pixel X dove inizia la scacchiera
extern int board_offset_y;   // pixel Y dove inizia la scacchiera
extern int sq_size;           // lato di una casa in pixel (dinamico)
extern int panel_x_dyn;       // X pannello laterale
extern int panel_w_dyn;       // larghezza pannello laterale

// ── Audio ────────────────────────────────────────────────────────────────────
extern Sound suono_move;
extern Sound suono_promozione;
extern Sound suono_capture;
extern Sound suono_scacco;
extern Sound suono_arrocco;

#pragma once
#include "raylib.h"
#include "variabili.h"

extern Color case_bianche, case_scure, selezionata;
extern Texture2D pedone_bianco, pedone_nero, cavallo_bianco, cavallo_nero;
extern Texture2D alfiere_bianco, alfiere_nero, torre_bianca, torre_nera;
extern Texture2D regina_bianca, regina_nera, re_bianco, re_nero, pallino_legale;

void disegna_scacchiera(int righe, int colonne, int lw, int lh, Color cb, Color cs);
void disegna_pezzi(scacchiera &b, int righe, int colonne, int lw, int lh,
                   Texture2D rb, Texture2D rn, Texture2D pb, Texture2D pn,
                   Texture2D cb, Texture2D cn, Texture2D ab, Texture2D an,
                   Texture2D tb, Texture2D tn, Texture2D qb, Texture2D qn);
void casa_selezionata(const int lw, const int lh, scacchiera &b,
                      int &col, int &riga, int &idx, int &pezzo,
                      int mossa_possibile[], bool &selezione);
void promozione_input(scacchiera &b, bool &promozione, int &turno, int idx_promozione);
void muovi_selezionato(bool &promozione, bool &mossa_fatta, bool selezione,
                       scacchiera &b, int &riga, int &col, int idx, int &pezzo,
                       int mossa_possibile[], int &mosse_possibili,
                       int &turno, int &avversario,
                       bool &enpassant, int &idx_enpassant,
                       bool re_e1, bool re_e8,
                       bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8,
                       bool check);
void inizializza_grafica();
void partita();
std::string fen_grafica();
void togli_evidenzia(int evidenziate[]);
bool torva_array(int evidenzia);

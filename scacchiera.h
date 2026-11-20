#pragma once
#include "pezzi.h"
#include <cstdint>
#include <vector>

class scacchiera
{
public:
    int casa[64];
    uint64_t bitboard_pieces[12];
    uint64_t occupancies[3];
    scacchiera();
    void updateBitboards();
    static int pieceIndex(int p);
};
void mosse_pseudoLegali(scacchiera &b, int &riga, int &col, int idx, int &pezzo, int mossa_possibile[], int &mosse_possibili, int &turno, int &avversario, bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8, bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8, int casa_re);
bool scacco_al_re(const scacchiera &b, int turno, int avversario, int &casa_re, bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8, bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8);
void esci_da_scacco(scacchiera b, int &casa_re, int avversario, int turno, int mossa_possibile[], int &mosse_possibili, int riga, int col, int idx, int pezzo, bool &matto, bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8, bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8);
bool esegui_mossa(scacchiera &b, int idx, int idx2, bool &matto);
void mosse_legali(scacchiera b, int &casa_re, int avversario, int turno, int mossa_possibile[], int &mosse_possibili, int riga, int col, int idx, int pezzo, bool &matto, bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8, bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8);
bool arrocco_si(const scacchiera &b, int turno, int avversario, int casa_re, bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8, bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8);
void fen();
int notazione_idx(int j);
bool scacco_matto(int turno, int avversario, scacchiera board_sm);
bool materiale_insufficente();
bool stelmate();
uint64_t calcolaHash(scacchiera &b, int turno);
bool pareggio();
bool ripetizione(scacchiera b, int turno);
void annulla_mossa(scacchiera &b);
void risultato_partita();
bool esegui_mossa_bot(scacchiera &b, int idx, int idx2, bool &matto);

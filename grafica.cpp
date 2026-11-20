#include "scacchiera.h"
#include "raylib.h"
#include "variabili.h"
#include "grafica.h"
#include <string>

Color case_bianche, case_scure, selezionata;
Texture2D pedone_bianco, pedone_nero;
Texture2D cavallo_bianco, cavallo_nero;
Texture2D alfiere_bianco, alfiere_nero;
Texture2D torre_bianca, torre_nera;
Texture2D regina_bianca, regina_nera;
Texture2D re_bianco, re_nero;
Texture2D pallino_legale;

void partita()
{
    if (!mossa)
    {
        pezzo = 0;
        idx = -1;
        mosse_possibili = 0;
        selezione = false;
        mossa = true;
    }
    if (!promozione && selezione && aggiorna_mosse)
    {
        mosse_possibili = 0;
        mosse_legali(b, casa_re, avversario, turno, mossa_possibile, mosse_possibili,
                     riga, col, idx, pezzo, matto, enpassant, idx_enpassant,
                     re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8);
        aggiorna_mosse = false;
    }
    if (promozione)
        promozione_input(b, promozione, turno, idx_promozione);
}

void inizializza_grafica()
{
    case_bianche = {237, 214, 176, 255};
    case_scure = {155, 88, 64, 255};
    pedone_bianco = LoadTexture("pezzi/wp.png");
    pedone_nero = LoadTexture("pezzi/bp.png");
    cavallo_bianco = LoadTexture("pezzi/wn.png");
    cavallo_nero = LoadTexture("pezzi/bn.png");
    alfiere_bianco = LoadTexture("pezzi/wb.png");
    alfiere_nero = LoadTexture("pezzi/bb.png");
    torre_bianca = LoadTexture("pezzi/wr.png");
    torre_nera = LoadTexture("pezzi/br.png");
    regina_bianca = LoadTexture("pezzi/wq.png");
    regina_nera = LoadTexture("pezzi/bq.png");
    re_bianco = LoadTexture("pezzi/wk.png");
    re_nero = LoadTexture("pezzi/bk.png");
    pallino_legale = LoadTexture("pezzi/pallino_legale.png");
}

void promozione_input(scacchiera &b, bool &promozione, int &turno, int idx_promozione)
{
    if (IsKeyPressed(KEY_R))
    {
        b.casa[idx_promozione] = pezzi::regina | turno;
        promozione = false;
    }
    else if (IsKeyPressed(KEY_T))
    {
        b.casa[idx_promozione] = pezzi::torre | turno;
        promozione = false;
    }
    else if (IsKeyPressed(KEY_A))
    {
        b.casa[idx_promozione] = pezzi::alfiere | turno;
        promozione = false;
    }
    else if (IsKeyPressed(KEY_C))
    {
        b.casa[idx_promozione] = pezzi::cavallo | turno;
        promozione = false;
    }
    if (!promozione)
        turno = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
}

void disegna_scacchiera(int, int, int, int, Color cb, Color cs)
{
    int ox = board_offset_x, oy = board_offset_y, sq = sq_size;
    for (int i = 0; i < 8; i++)
        for (int c = 0; c < 8; c++)
            DrawRectangle(ox + c * sq, oy + i * sq, sq, sq, ((i + c) % 2 == 0) ? cb : cs);
    int fs = (sq >= 72) ? 11 : 8;
    Color ll = {155, 88, 64, 200}, ld = {237, 214, 176, 200};
    for (int c = 0; c < 8; c++)
    {
        char buf[2] = {(char)('a' + c), 0};
        bool li = ((7 + c) % 2 == 0);
        DrawText(buf, ox + c * sq + sq - fs - 2, oy + 7 * sq + sq - fs - 2, fs, li ? ll : ld);
    }
    for (int i = 0; i < 8; i++)
    {
        char buf[2] = {(char)('8' - i), 0};
        bool li = (i % 2 == 0);
        DrawText(buf, ox + 2, oy + i * sq + 2, fs, li ? ll : ld);
    }
}

void disegna_pezzi(scacchiera &b, int righe, int colonne, int, int,
                   Texture2D rb, Texture2D rn, Texture2D pb, Texture2D pn,
                   Texture2D cb, Texture2D cn, Texture2D ab, Texture2D an,
                   Texture2D tb, Texture2D tn, Texture2D qb, Texture2D qn)
{
    int ox = board_offset_x, oy = board_offset_y, sq = sq_size;
    for (int i = 0; i < righe; i++)
        for (int c = 0; c < colonne; c++)
        {
            int p = b.casa[i * colonne + c], tipo = p & 7;
            bool bia = (p & pezzi::bianco) != 0;
            Texture2D *tex = nullptr;
            if (tipo == pezzi::re)
                tex = bia ? &rb : &rn;
            else if (tipo == pezzi::pedone)
                tex = bia ? &pb : &pn;
            else if (tipo == pezzi::cavallo)
                tex = bia ? &cb : &cn;
            else if (tipo == pezzi::alfiere)
                tex = bia ? &ab : &an;
            else if (tipo == pezzi::regina)
                tex = bia ? &qb : &qn;
            else if (tipo == pezzi::torre)
                tex = bia ? &tb : &tn;
            if (tex)
            {
                Rectangle src = {0, 0, (float)tex->width, (float)tex->height};
                Rectangle dst = {(float)(ox + c * sq), (float)(oy + i * sq), (float)sq, (float)sq};
                DrawTexturePro(*tex, src, dst, {0, 0}, 0.0f, WHITE);
            }
        }
}

void casa_selezionata(const int, const int, scacchiera &b, int &col, int &riga,
                      int &idx, int &pezzo, int mossa_possibile[], bool &selezione)
{
    Vector2 m = GetMousePosition();
    int sq = sq_size;
    col = (int)((m.x - board_offset_x) / sq);
    riga = (int)((m.y - board_offset_y) / sq);
    if (riga >= 0 && riga < 8 && col >= 0 && col < 8)
    {
        idx = riga * 8 + col;
        pezzo = b.casa[idx];
        for (int i = 0; i < 27; i++)
            mossa_possibile[i] = -1;
        mosse_possibili = 0;
        selezione = true;
    }
}

void muovi_selezionato(bool &promozione, bool &mossa_fatta, bool selezione,
                       scacchiera &b, int &riga, int &col, int idx, int &pezzo,
                       int mossa_possibile[], int &mosse_possibili, int &turno, int &avversario,
                       bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8,
                       bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8, bool check)
{
    mossa_fatta = false;
    if (!selezione)
        return;
    Vector2 m = GetMousePosition();
    int sq = sq_size;
    int cc = (int)((m.x - board_offset_x) / sq), rc = (int)((m.y - board_offset_y) / sq);
    if (rc >= 0 && rc < 8 && cc >= 0 && cc < 8)
    {
        int idx2 = rc * 8 + cc;
        mosse_legali(b, casa_re, avversario, turno, mossa_possibile, mosse_possibili,
                     riga, col, idx, pezzo, matto, enpassant, idx_enpassant,
                     re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8);
        for (int i = 0; i < mosse_possibili; i++)
            if (idx2 == mossa_possibile[i])
            {
                esegui_mossa(b, idx, idx2, matto);
                mossa_fatta = true;
                selezione = false;
                break;
            }
    }
}

std::string fen_grafica() { return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR wKQkq"; }
void togli_evidenzia(int ev[])
{
    for (int i = 0; i < 64; i++)
        ev[i] = -1;
    quante_evid = 0;
}
bool torva_array(int ev)
{
    for (int i = 0; i < quante_evid; i++)
        if (evidenziate[i] == ev)
            return true;
    return false;
}
void evidenzia_casa(bool) {}
void disegna_eval(int) {}

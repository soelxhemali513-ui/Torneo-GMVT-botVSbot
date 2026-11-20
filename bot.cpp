#include "variabili.h"
#include "scacchiera.h"
#include "pezzi.h"
#include "bot.h"
#include "aperture.h"
#include <cstdlib>
#include <utility>
#include <algorithm>
#include <chrono>
#include <cstring>

int valore_pezzi::pedone = 100;
int valore_pezzi::cavallo = 320;
int valore_pezzi::alfiere = 330;
int valore_pezzi::torre = 500;
int valore_pezzi::regina = 900;
int valore_pezzi::re = 20000;

int DEPTH = 6;

int temperamento_bianco = 0;
int temperamento_nero = 0;

static const int INFINITO = 1000000;

EntrataTT tabella_trasposizioni[DIMENSIONE_TT];
int mosse_killer[PROFONDITA_MASSIMA][2];
int tabella_storia[2][64][64];

static const int pedone_mg[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    98, 134, 61, 95, 68, 126, 34, -11,
    -6, 7, 26, 31, 65, 56, 25, -20,
    -14, 13, 6, 21, 23, 12, 17, -23,
    -27, -2, -5, 12, 17, 6, 10, -25,
    -26, -4, -4, -10, 3, 3, 33, -12,
    -35, -1, -20, -23, -15, 24, 38, -22,
    0, 0, 0, 0, 0, 0, 0, 0};
static const int pedone_eg[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    178, 173, 158, 134, 147, 132, 165, 187,
    94, 100, 85, 67, 56, 53, 82, 84,
    32, 24, 13, 5, -2, 4, 17, 17,
    13, 9, -3, -7, -7, -8, 3, -1,
    4, 7, -6, 1, 0, -5, -1, -8,
    13, 8, 8, 10, 13, 0, 2, -7,
    0, 0, 0, 0, 0, 0, 0, 0};
static const int cavallo_mg[64] = {
    -167, -89, -34, -49, 61, -97, -15, -107,
    -73, -41, 72, 36, 23, 62, 7, -17,
    -47, 60, 37, 65, 84, 129, 73, 44,
    -9, 17, 19, 53, 37, 69, 18, 22,
    -13, 4, 16, 13, 28, 19, 21, -8,
    -23, -9, 12, 10, 19, 17, 25, -16,
    -29, -53, -12, -3, -1, 18, -14, -19,
    -105, -21, -58, -33, -17, -28, -19, -23};
static const int cavallo_eg[64] = {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25, -8, -25, -2, -9, -25, -24, -52,
    -24, -20, 10, 9, -1, -9, -19, -41,
    -17, 3, 22, 22, 22, 11, 8, -18,
    -18, -6, 16, 25, 16, 17, 4, -18,
    -23, -3, -1, 15, 10, -3, -20, -22,
    -42, -20, -10, -5, -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64};
static const int alfiere_mg[64] = {
    -29, 4, -82, -37, -25, -42, 7, -8,
    -26, 16, -18, -13, 30, 59, 18, -47,
    -16, 37, 43, 40, 35, 50, 37, -2,
    -4, 5, 19, 50, 37, 37, 7, -2,
    -6, 13, 13, 26, 34, 12, 10, 4,
    0, 15, 15, 15, 14, 27, 18, 10,
    4, 15, 16, 0, 7, 21, 33, 1,
    -33, -3, -14, -21, -13, -12, -39, -21};
static const int alfiere_eg[64] = {
    -14, -21, -11, -8, -7, -9, -17, -24,
    -8, -4, 7, -12, -3, -13, -4, -14,
    2, -8, 0, -1, -2, 6, 0, 4,
    -3, 9, 12, 9, 14, 10, 3, 2,
    -6, 3, 13, 19, 7, 10, -3, -9,
    -12, -3, 8, 10, 13, 3, -7, -15,
    -14, -18, -7, -1, 4, -9, -15, -27,
    -23, -9, -23, -5, -9, -16, -5, -17};
static const int torre_mg[64] = {
    32, 42, 32, 51, 63, 9, 31, 43,
    27, 32, 58, 62, 80, 67, 26, 44,
    -5, 19, 26, 36, 17, 45, 61, 16,
    -24, -11, 7, 26, 24, 35, -8, -20,
    -36, -26, -12, -1, 9, -7, 6, -23,
    -45, -25, -16, -17, 3, 0, -5, -33,
    -44, -16, -20, -9, -1, 11, -6, -71,
    -19, -13, 1, 17, 16, 7, -37, -26};
static const int torre_eg[64] = {
    13, 10, 18, 15, 12, 12, 8, 5,
    11, 13, 13, 11, -3, 3, 8, 3,
    7, 7, 7, 5, 4, -3, -5, -3,
    4, 3, 13, 1, 2, 1, -1, 2,
    3, 5, 8, 4, -5, -6, -8, -11,
    -4, 0, -5, -1, -7, -12, -8, -16,
    -6, -6, 0, 2, -9, -9, -11, -3,
    -9, 2, 3, -1, -5, -13, 4, -20};
static const int regina_mg[64] = {
    -28, 0, 29, 12, 59, 44, 43, 45,
    -24, -39, -5, 1, -16, 57, 28, 54,
    -13, -17, 7, 8, 29, 56, 47, 57,
    -27, -27, -16, -16, -1, 17, -2, 1,
    -9, -26, -9, -10, -2, -4, 3, -3,
    -14, 2, -11, -2, -5, 2, 14, 5,
    -35, -8, 11, 2, 8, 15, -3, 1,
    -1, -18, -9, 10, -15, -25, -31, -50};
static const int regina_eg[64] = {
    -9, 22, 22, 27, 27, 19, 10, 20,
    -17, 20, 32, 41, 58, 25, 30, 0,
    -20, 6, 9, 49, 47, 35, 19, 9,
    3, 22, 24, 45, 57, 40, 57, 36,
    -18, 28, 19, 47, 31, 34, 39, 23,
    -16, -27, 15, 6, 9, 17, 10, 5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43, -5, -32, -20, -41};
static const int re_mg[64] = {
    -65, 23, 16, -15, -56, -34, 2, 13,
    29, -1, -20, -7, -8, -4, -38, -29,
    -9, 24, 2, -16, -20, 6, 22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49, -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
    1, 7, -8, -64, -43, -16, 9, 8,
    -15, 36, 12, -54, 8, -28, 24, 14};
static const int re_eg[64] = {
    -74, -35, -18, -18, -11, 15, 4, -17,
    -12, 17, 14, 17, 17, 38, 23, 11,
    10, 17, 23, 15, 20, 45, 44, 13,
    -8, 22, 24, 27, 26, 33, 26, 3,
    -18, -4, 21, 24, 27, 23, 9, -11,
    -19, -3, 11, 21, 23, 16, 7, -9,
    -27, -11, 4, 13, 14, 4, -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43};

static const int *tabella_mg[6] = {pedone_mg, cavallo_mg, alfiere_mg, torre_mg, regina_mg, re_mg};
static const int *tabella_eg[6] = {pedone_eg, cavallo_eg, alfiere_eg, torre_eg, regina_eg, re_eg};

static int fase_gioco(scacchiera &scacch)
{
    static const int pesi[7] = {0, 0, 1, 1, 2, 4, 0};
    int fase = 24;
    for (int i = 0; i < 64; i++)
    {
        int p = scacch.casa[i];
        if (p == 0)
            continue;
        int tipo = p & 7;
        if (tipo >= 1 && tipo <= 5)
            fase -= pesi[tipo];
    }
    if (fase < 0)
        fase = 0;
    return (fase * 256 + 12) / 24;
}

int valore_pezzo(int pezzo)
{
    if (pezzo == 0)
        return 0;
    switch (pezzo & 7)
    {
    case pezzi::pedone:
        return valore_pezzi::pedone;
    case pezzi::cavallo:
        return valore_pezzi::cavallo;
    case pezzi::alfiere:
        return valore_pezzi::alfiere;
    case pezzi::torre:
        return valore_pezzi::torre;
    case pezzi::regina:
        return valore_pezzi::regina;
    case pezzi::re:
        return valore_pezzi::re;
    }
    return 0;
}

static int king_safety_score(scacchiera &scacch, int colore_re, int colore_nemico)
{
    int casella_re = -1;
    for (int i = 0; i < 64; i++)
    {
        if (scacch.casa[i] == (pezzi::re | colore_re))
        {
            casella_re = i;
            break;
        }
    }
    if (casella_re == -1)
        return 0;

    int riga_re = casella_re / 8;
    int col_re = casella_re % 8;
    int penalita = 0;

    for (int dr = -1; dr <= 1; dr++)
    {
        for (int dc = -1; dc <= 1; dc++)
        {
            if (dr == 0 && dc == 0)
                continue;
            int r2 = riga_re + dr;
            int c2 = col_re + dc;
            if (r2 < 0 || r2 > 7 || c2 < 0 || c2 > 7)
                continue;
            int casella_vicina = r2 * 8 + c2;

            for (int j = 0; j < 64; j++)
            {
                int p = scacch.casa[j];
                if ((p & 24) != colore_nemico)
                    continue;
                int tipo = p & 7;
                int rj = j / 8, cj = j % 8;

                bool attacca = false;
                switch (tipo)
                {
                case pezzi::pedone:
                {
                    int dir = (colore_nemico == pezzi::bianco) ? -1 : 1;
                    if (rj + dir == r2 && (cj - 1 == c2 || cj + 1 == c2))
                        attacca = true;
                    break;
                }
                case pezzi::cavallo:
                {
                    int dr2 = abs(rj - r2), dc2 = abs(cj - c2);
                    if ((dr2 == 2 && dc2 == 1) || (dr2 == 1 && dc2 == 2))
                        attacca = true;
                    break;
                }
                case pezzi::alfiere:
                {
                    if (abs(rj - r2) == abs(cj - c2) && abs(rj - r2) > 0)
                    {
                        int sr = (r2 > rj) ? 1 : -1, sc = (c2 > cj) ? 1 : -1;
                        bool libero = true;
                        for (int step = 1; rj + sr * step != r2; step++)
                            if (scacch.casa[(rj + sr * step) * 8 + (cj + sc * step)] != 0)
                            {
                                libero = false;
                                break;
                            }
                        if (libero)
                            attacca = true;
                    }
                    break;
                }
                case pezzi::torre:
                {
                    if (rj == r2 || cj == c2)
                    {
                        int sr = (r2 == rj) ? 0 : (r2 > rj ? 1 : -1);
                        int sc = (c2 == cj) ? 0 : (c2 > cj ? 1 : -1);
                        bool libero = true;
                        int cr = rj + sr, cc = cj + sc;
                        while (cr != r2 || cc != c2)
                        {
                            if (scacch.casa[cr * 8 + cc] != 0)
                            {
                                libero = false;
                                break;
                            }
                            cr += sr;
                            cc += sc;
                        }
                        if (libero)
                            attacca = true;
                    }
                    break;
                }
                case pezzi::regina:
                {
                    bool diag = (abs(rj - r2) == abs(cj - c2) && abs(rj - r2) > 0);
                    bool retta = (rj == r2 || cj == c2);
                    if (diag || retta)
                    {
                        int sr = (r2 == rj) ? 0 : (r2 > rj ? 1 : -1);
                        int sc = (c2 == cj) ? 0 : (c2 > cj ? 1 : -1);
                        bool libero = true;
                        int cr = rj + sr, cc = cj + sc;
                        while (cr != r2 || cc != c2)
                        {
                            if (scacch.casa[cr * 8 + cc] != 0)
                            {
                                libero = false;
                                break;
                            }
                            cr += sr;
                            cc += sc;
                        }
                        if (libero)
                            attacca = true;
                    }
                    break;
                }
                }
                if (attacca)
                {
                    penalita += 15;
                    break;
                }
            }
        }
    }

    for (int j = 0; j < 64; j++)
    {
        int p = scacch.casa[j];
        if ((p & 24) != colore_nemico)
            continue;
        int tipo = p & 7;
        int rj = j / 8, cj = j % 8;
        if (tipo == pezzi::regina || tipo == pezzi::torre)
        {
            if (rj == riga_re || cj == col_re)
                penalita += 10;
        }
    }

    return -penalita;
}

static int valutazione_posizionale(scacchiera &scacch, int &mg, int &eg)
{
    int bonus = 0;

    int pedoni_b[8] = {0}, pedoni_n[8] = {0};
    for (int i = 0; i < 64; i++)
    {
        int p = scacch.casa[i];
        if ((p & 7) != pezzi::pedone)
            continue;
        if ((p & 24) == pezzi::bianco)
            pedoni_b[i % 8]++;
        else
            pedoni_n[i % 8]++;
    }

    for (int i = 0; i < 64; i++)
    {
        int p = scacch.casa[i];
        if (p == 0)
            continue;
        int tipo = p & 7;
        int col = i % 8;
        int riga = i / 8;
        bool bianco = ((p & 24) == pezzi::bianco);
        int segno = bianco ? 1 : -1;

        if (tipo == pezzi::pedone)
        {
            int *amici = bianco ? pedoni_b : pedoni_n;
            int *nemici = bianco ? pedoni_n : pedoni_b;
            if (amici[col] > 1)
            {
                mg -= segno * 12;
                eg -= segno * 20;
            }
            bool isolato = true;
            if (col > 0 && amici[col - 1] > 0)
                isolato = false;
            if (col < 7 && amici[col + 1] > 0)
                isolato = false;
            if (isolato)
            {
                mg -= segno * 15;
                eg -= segno * 25;
            }
            if (isolato)
            {
                int front = bianco ? i - 8 : i + 8;
                if (front >= 0 && front < 64 && scacch.casa[front] != 0)
                {
                    mg -= segno * 10;
                    eg -= segno * 15;
                }
            }
            bool passato = true;
            int dir = bianco ? -1 : 1;
            for (int r = riga + dir; r >= 0 && r < 8; r += dir)
            {
                if (col > 0 && (scacch.casa[r * 8 + col - 1] & 7) == pezzi::pedone && (scacch.casa[r * 8 + col - 1] & 24) != (p & 24))
                {
                    passato = false;
                    break;
                }
                if ((scacch.casa[r * 8 + col] & 7) == pezzi::pedone && (scacch.casa[r * 8 + col] & 24) != (p & 24))
                {
                    passato = false;
                    break;
                }
                if (col < 7 && (scacch.casa[r * 8 + col + 1] & 7) == pezzi::pedone && (scacch.casa[r * 8 + col + 1] & 24) != (p & 24))
                {
                    passato = false;
                    break;
                }
            }
            if (passato)
            {
                int avanzamento = bianco ? (7 - riga) : riga;
                static const int bonus_passato[8] = {0, 10, 20, 35, 55, 80, 110, 0};
                mg += segno * bonus_passato[avanzamento] / 2;
                eg += segno * bonus_passato[avanzamento];
            }
        }

        if (tipo == pezzi::torre)
        {
            int *amici = bianco ? pedoni_b : pedoni_n;
            int *nemici = bianco ? pedoni_n : pedoni_b;
            if (amici[col] == 0 && nemici[col] == 0)
            {
                mg += segno * 20;
                eg += segno * 15;
            }
            else if (amici[col] == 0)
            {
                mg += segno * 10;
                eg += segno * 8;
            }
            int settima = bianco ? 1 : 6;
            if (riga == settima)
            {
                mg += segno * 30;
                eg += segno * 50;
            }
        }
        if (tipo == pezzi::cavallo)
        {
            int *nemici = bianco ? pedoni_n : pedoni_b;
            bool outpost = true;
            if (col > 0 && nemici[col - 1] > 0)
            {
                int dir_nem = bianco ? 1 : -1;
                for (int r = riga + dir_nem; r >= 0 && r < 8; r += dir_nem)
                    if ((scacch.casa[r * 8 + col - 1] & 7) == pezzi::pedone && (scacch.casa[r * 8 + col - 1] & 24) != (p & 24))
                    {
                        outpost = false;
                        break;
                    }
            }
            if (outpost && col < 7 && nemici[col + 1] > 0)
            {
                int dir_nem = bianco ? 1 : -1;
                for (int r = riga + dir_nem; r >= 0 && r < 8; r += dir_nem)
                    if ((scacch.casa[r * 8 + col + 1] & 7) == pezzi::pedone && (scacch.casa[r * 8 + col + 1] & 24) != (p & 24))
                    {
                        outpost = false;
                        break;
                    }
            }
            if (outpost)
            {
                int centro = bianco ? (7 - riga) : riga;
                if (centro >= 3)
                {
                    mg += segno * 25;
                    eg += segno * 15;
                }
                else
                {
                    mg += segno * 10;
                    eg += segno * 5;
                }
            }
            if (col == 0 || col == 7)
            {
                mg -= segno * 20;
                eg -= segno * 20;
            }
            if (riga == 0 || riga == 7)
            {
                mg -= segno * 10;
                eg -= segno * 10;
            }
        }
        if (tipo == pezzi::cavallo)
        {
            static const int salti[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
            int mob = 0;
            for (auto &s : salti)
            {
                int nr = riga + s[0], nc = col + s[1];
                if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8)
                {
                    int dest = scacch.casa[nr * 8 + nc];
                    if (dest == 0 || (dest & 24) != (p & 24))
                        mob++;
                }
            }
            mg += segno * (mob - 4) * 4;
            eg += segno * (mob - 4) * 4;
        }
        if (tipo == pezzi::alfiere)
        {
            static const int dirs[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
            int mob = 0;
            for (auto &d : dirs)
            {
                for (int step = 1; step < 8; step++)
                {
                    int nr = riga + d[0] * step, nc = col + d[1] * step;
                    if (nr < 0 || nr > 7 || nc < 0 || nc > 7)
                        break;
                    int dest = scacch.casa[nr * 8 + nc];
                    if (dest == 0)
                    {
                        mob++;
                        continue;
                    }
                    if ((dest & 24) != (p & 24))
                        mob++;
                    break;
                }
            }
            mg += segno * (mob - 7) * 3;
            eg += segno * (mob - 7) * 3;
        }
        if (tipo == pezzi::torre)
        {
            static const int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
            int mob = 0;
            for (auto &d : dirs)
            {
                for (int step = 1; step < 8; step++)
                {
                    int nr = riga + d[0] * step, nc = col + d[1] * step;
                    if (nr < 0 || nr > 7 || nc < 0 || nc > 7)
                        break;
                    int dest = scacch.casa[nr * 8 + nc];
                    if (dest == 0)
                    {
                        mob++;
                        continue;
                    }
                    if ((dest & 24) != (p & 24))
                        mob++;
                    break;
                }
            }
            mg += segno * (mob - 7) * 2;
            eg += segno * (mob - 7) * 3;
        }
        if (tipo == pezzi::regina)
        {
            static const int dirs[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
            int mob = 0;
            for (auto &d : dirs)
            {
                for (int step = 1; step < 8; step++)
                {
                    int nr = riga + d[0] * step, nc = col + d[1] * step;
                    if (nr < 0 || nr > 7 || nc < 0 || nc > 7)
                        break;
                    int dest = scacch.casa[nr * 8 + nc];
                    if (dest == 0)
                    {
                        mob++;
                        continue;
                    }
                    if ((dest & 24) != (p & 24))
                        mob++;
                    break;
                }
            }
            mg += segno * (mob - 14);
            eg += segno * (mob - 14) * 2;
        }
    }
    return bonus;
}

int valutazione(scacchiera &scacch, int colore_bot)
{
    int fase = fase_gioco(scacch);
    int mg = 0, eg = 0;

    for (int casella = 0; casella < 64; casella++)
    {
        int pezzo = scacch.casa[casella];
        if (pezzo == 0)
            continue;
        int tipo = (pezzo & 7) - 1;
        bool e_bianco = ((pezzo & 24) == pezzi::bianco);
        int indice = e_bianco ? casella : (63 - casella);
        int segno = e_bianco ? 1 : -1;
        int materiale = valore_pezzo(pezzo);
        mg += segno * (materiale + tabella_mg[tipo][indice]);
        eg += segno * (materiale + tabella_eg[tipo][indice]);
    }

    int alfieri_bianchi = 0, alfieri_neri = 0;
    for (int casella = 0; casella < 64; casella++)
    {
        int pezzo = scacch.casa[casella];
        if ((pezzo & 7) == pezzi::alfiere)
        {
            if ((pezzo & 24) == pezzi::bianco)
                alfieri_bianchi++;
            else
                alfieri_neri++;
        }
    }
    if (alfieri_bianchi >= 2)
    {
        mg += 30;
        eg += 50;
    }
    if (alfieri_neri >= 2)
    {
        mg -= 30;
        eg -= 50;
    }

    if (fase > 100)
    {
        for (int casella = 0; casella < 64; casella++)
        {
            int pezzo = scacch.casa[casella];
            if ((pezzo & 7) != pezzi::pedone)
                continue;
            bool e_bianco = ((pezzo & 24) == pezzi::bianco);
            int riga = casella / 8;
            int avanzamento;
            if (e_bianco)
                avanzamento = 7 - riga;
            else
                avanzamento = riga;
            int bonus = (avanzamento * avanzamento * fase) / 512;
            if (e_bianco)
            {
                mg += bonus;
                eg += bonus;
            }
            else
            {
                mg -= bonus;
                eg -= bonus;
            }
        }
    }

    valutazione_posizionale(scacch, mg, eg);
    int ks_bianco = king_safety_score(scacch, pezzi::bianco, pezzi::nero);
    int ks_nero = king_safety_score(scacch, pezzi::nero, pezzi::bianco);
    int peso_ks = (256 - fase);
    mg += (ks_bianco - ks_nero) * peso_ks / 128;

    int punteggio = (mg * (256 - fase) + eg * fase) / 256;
    if (colore_bot == pezzi::nero)
        punteggio = -punteggio;
    return punteggio;
}

int vittima_attaccante(int attaccante, int vittima)
{
    static const int val[7] = {0, 1, 2, 3, 4, 5, 6};
    return val[vittima & 7] * 10 - val[attaccante & 7];
}

void aggiorna_killer(int livello, int da, int a)
{
    if (livello >= PROFONDITA_MASSIMA)
        return;
    int chiave = da * 64 + a;
    if (mosse_killer[livello][0] != chiave)
    {
        mosse_killer[livello][1] = mosse_killer[livello][0];
        mosse_killer[livello][0] = chiave;
    }
}

void ordina_mosse(Mossa mosse[], int contatore, int livello, int tt_da, int tt_a, scacchiera &scacch)
{
    // -- Move ordering algorithm (TT move, MVV-LVA, killer heuristic, history heuristic)
    int indice_colore = (turno == pezzi::bianco) ? 0 : 1;
    for (int i = 0; i < contatore; i++)
    {
        int da = mosse[i].da;
        int a = mosse[i].a;
        int vittima = scacch.casa[a];
        int attacc = scacch.casa[da];
        if (da == tt_da && a == tt_a)
        {
            mosse[i].punteggio = 2000000;
        }
        else if (vittima != 0)
        {
            mosse[i].punteggio = 1000000 + vittima_attaccante(attacc, vittima);
        }
        else if (livello < PROFONDITA_MASSIMA && mosse_killer[livello][0] == da * 64 + a)
        {
            mosse[i].punteggio = 900000;
        }
        else if (livello < PROFONDITA_MASSIMA && mosse_killer[livello][1] == da * 64 + a)
        {
            mosse[i].punteggio = 800000;
        }
        else
        {
            mosse[i].punteggio = tabella_storia[indice_colore][da][a];
        }
    }
    std::sort(mosse, mosse + contatore, [](const Mossa &x, const Mossa &y)
              { return x.punteggio > y.punteggio; });
}

int genera_mosse(Mossa mosse[], int turno_loc, scacchiera scacch)
{
    int contatore = 0;
    int avversario_loc = (turno_loc == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
    int riga_promozione = (turno_loc == pezzi::bianco) ? 0 : 7;

    // Calcola la posizione del re localmente — non usare il globale casa_re
    // che può essere stale durante la ricerca quando il re si è mosso
    int casa_re_loc = -1;
    for (int i = 0; i < 64; i++)
    {
        if (scacch.casa[i] == (pezzi::re | turno_loc))
        {
            casa_re_loc = i;
            break;
        }
    }
    if (casa_re_loc == -1)
        return 0; // posizione invalida, re non trovato

    for (int i = 0; i < 64; i++)
    {
        if ((scacch.casa[i] & 24) != turno_loc)
            continue;
        int riga_loc = i / 8;
        int col_loc = i % 8;
        int pezzo_loc = scacch.casa[i];
        int numero_mosse = 0;
        int mosse_possibili_loc[27];
        for (int j = 0; j < 27; j++)
            mosse_possibili_loc[j] = -1;
        mosse_legali(scacch, casa_re_loc, avversario_loc, turno_loc,
                     mosse_possibili_loc, numero_mosse,
                     riga_loc, col_loc, i, pezzo_loc,
                     matto, enpassant, idx_enpassant,
                     re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8);
        for (int j = 0; j < numero_mosse; j++)
        {
            int dest = mosse_possibili_loc[j];
            if (dest == -1 || contatore >= 252)
                continue;
            bool e_promozione = ((pezzo_loc & 7) == pezzi::pedone) && (dest / 8 == riga_promozione);
            if (e_promozione)
            {
                int pezzi_promo[4] = {pezzi::regina, pezzi::torre, pezzi::alfiere, pezzi::cavallo};
                for (int k = 0; k < 4; k++)
                {
                    mosse[contatore].da = i;
                    mosse[contatore].a = dest;
                    mosse[contatore].punteggio = 0;
                    mosse[contatore].promozione = pezzi_promo[k];
                    contatore++;
                }
            }
            else
            {
                mosse[contatore].da = i;
                mosse[contatore].a = dest;
                mosse[contatore].punteggio = 0;
                mosse[contatore].promozione = 0;
                contatore++;
            }
        }
    }
    return contatore;
}

int ricerca_quiete(scacchiera &scacch, int alfa, int beta, int colore_bot, int profondita_qs)
{
    nodes_searched++;

    // Limite profondità ricerca quiete: evita stack overflow in posizioni tattiche esplosive
    if (profondita_qs >= 8)
        return valutazione(scacch, turno);

    int stand_pat = valutazione(scacch, turno);
    if (stand_pat >= beta)
        return beta;
    static const int DELTA = 950;
    if (stand_pat < alfa - DELTA)
        return alfa;
    if (stand_pat > alfa)
        alfa = stand_pat;

    Mossa tutte[256];
    int totale = genera_mosse(tutte, turno, scacch);

    Mossa tattiche[96];
    int numero_tattiche = 0;
    for (int i = 0; i < totale; i++)
    {
        bool e_cattura = (scacch.casa[tutte[i].a] != 0);
        bool e_promozione = (tutte[i].promozione != 0);
        if (!e_cattura && !e_promozione)
            continue;
        int vittima = scacch.casa[tutte[i].a];
        int attaccante = scacch.casa[tutte[i].da];
        int bonus_promo = e_promozione ? 100 : 0;
        tutte[i].punteggio = bonus_promo + (e_cattura ? vittima_attaccante(attaccante, vittima) : 0);
        tattiche[numero_tattiche++] = tutte[i];
    }
    std::sort(tattiche, tattiche + numero_tattiche, [](const Mossa &x, const Mossa &y)
              { return x.punteggio > y.punteggio; });

    for (int i = 0; i < numero_tattiche; i++)
    {
        int turno_prima_mossa = turno;
        bool matto_loc = false;
        esegui_mossa(scacch, tattiche[i].da, tattiche[i].a, matto_loc);
        if (tattiche[i].promozione != 0 && promozione)
        {
            scacch.casa[idx_promozione] = tattiche[i].promozione | turno_prima_mossa;
            promozione = false;
            turno = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
            avversario = (avversario == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
        }
        int risultato = -ricerca_quiete(scacch, -beta, -alfa, colore_bot, profondita_qs + 1);
        annulla_mossa(scacch);
        if (risultato >= beta)
            return beta;
        if (risultato > alfa)
            alfa = risultato;
    }
    return alfa;
}

int minimax(scacchiera &scacch, int profondita, int alfa, int beta, int colore_bot, int livello)
{
    nodes_searched++;
    const int MATE_SCORE = 30000;
    int alfa_originale = alfa;

    uint64_t hash = calcolaHash(scacch, turno);
    int tt_da = -1, tt_a = -1;
    EntrataTT &entrata = tabella_trasposizioni[hash & (DIMENSIONE_TT - 1)];
    if (entrata.hash == hash)
    {
        tt_da = entrata.migliore_da;
        tt_a = entrata.migliore_a;
        if (entrata.profondita >= profondita)
        {
            if (entrata.bandiera == TT_ESATTO)
                return entrata.valore;
            if (entrata.bandiera == TT_ALFA && entrata.valore <= alfa)
                return entrata.valore;
            if (entrata.bandiera == TT_BETA && entrata.valore >= beta)
                return entrata.valore;
        }
    }

    if (profondita <= 0)
        return ricerca_quiete(scacch, alfa, beta, colore_bot, 0);

    int avv = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
    bool in_scacco = scacco_al_re(scacch, turno, avv, casa_re, enpassant, idx_enpassant,
                                  re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8);

    if (profondita == 1 && !in_scacco)
    {
        int static_eval = valutazione(scacch, turno);
        const int margine_futilita = 120;
        if (static_eval + margine_futilita <= alfa)
            return alfa;
    }

    if (!in_scacco && profondita >= 3 && livello > 0 && fase_gioco(scacch) < 200)
    {
        int turno_prec = turno;
        int avversario_prec = avversario;
        turno = avv;
        avversario = turno_prec;
        int val_null = -minimax(scacch, profondita - 1 - 2, -beta, -beta + 1, colore_bot, livello + 1);
        turno = turno_prec;
        avversario = avversario_prec;
        if (val_null >= beta)
            return beta;
    }

    Mossa mosse[256];
    int numero_mosse = genera_mosse(mosse, turno, scacch);
    if (numero_mosse == 0)
    {
        if (in_scacco)
            return -MATE_SCORE + livello;
        return 0;
    }

    ordina_mosse(mosse, numero_mosse, livello, tt_da, tt_a, scacch);

    int migliore = -INFINITO;
    int migl_da = mosse[0].da;
    int migl_a = mosse[0].a;

    for (int i = 0; i < numero_mosse; i++)
    {
        bool e_cattura = (scacch.casa[mosse[i].a] != 0);
        bool e_promozione_mossa = (mosse[i].promozione != 0);
        int turno_prima_mossa = turno;
        bool matto_loc = false;
        esegui_mossa(scacch, mosse[i].da, mosse[i].a, matto_loc);
        if (e_promozione_mossa && promozione)
        {
            scacch.casa[idx_promozione] = mosse[i].promozione | turno_prima_mossa;
            promozione = false;
            turno = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
            avversario = (avversario == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
        }

        int estensione = 0;
        int colore_avv_ext = (turno_prima_mossa == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
        bool da_scacco = scacco_al_re(scacch, colore_avv_ext, turno_prima_mossa,
                                      casa_re, enpassant, idx_enpassant,
                                      re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8);
        if (da_scacco || e_promozione_mossa)
            estensione = 1;

        int risultato;
        if (i >= 4 && profondita >= 3 && !e_cattura && !e_promozione_mossa && !in_scacco && !da_scacco && estensione == 0)
        {
            int riduzione = (i >= 8) ? 2 : 1;
            risultato = -minimax(scacch, profondita - 1 - riduzione, -alfa - 1, -alfa, colore_bot, livello + 1);
            if (risultato > alfa)
            {
                risultato = -minimax(scacch, profondita - 1 + estensione, -beta, -alfa, colore_bot, livello + 1);
            }
        }
        else
        {
            risultato = -minimax(scacch, profondita - 1 + estensione, -beta, -alfa, colore_bot, livello + 1);
        }

        annulla_mossa(scacch);

        if (risultato > migliore)
        {
            migliore = risultato;
            migl_da = mosse[i].da;
            migl_a = mosse[i].a;
        }
        if (risultato > alfa)
            alfa = risultato;

        if (alfa >= beta)
        {
            if (!e_cattura)
            {
                aggiorna_killer(livello, mosse[i].da, mosse[i].a);
                int ci = (turno_prima_mossa == pezzi::bianco) ? 0 : 1;
                tabella_storia[ci][mosse[i].da][mosse[i].a] += profondita * profondita;
            }
            break;
        }
    }

    BandieraTT bandiera = TT_ESATTO;
    if (migliore <= alfa_originale)
        bandiera = TT_ALFA;
    else if (migliore >= beta)
        bandiera = TT_BETA;

    entrata.hash = hash;
    entrata.profondita = profondita;
    entrata.valore = migliore;
    entrata.bandiera = bandiera;
    entrata.migliore_da = migl_da;
    entrata.migliore_a = migl_a;
    return migliore;
}

int pezzo_promozione_scelto = 0;

int scegli_promozione(scacchiera &scacch, int casella, int colore_bot)
{
    int candidati[4] = {pezzi::regina, pezzi::torre, pezzi::alfiere, pezzi::cavallo};
    int miglior_pezzo = pezzi::regina;
    int miglior_val = -INFINITO;
    int colore = (colore_bot == pezzi::bianco) ? pezzi::bianco : pezzi::nero;
    for (int k = 0; k < 4; k++)
    {
        scacch.casa[casella] = candidati[k] | colore;
        int val = valutazione(scacch, colore_bot);
        if (val > miglior_val)
        {
            miglior_val = val;
            miglior_pezzo = candidati[k];
        }
    }
    scacch.casa[casella] = pezzi::pedone | colore;
    return miglior_pezzo;
}

std::pair<int, int> valuta_mossa()
{
    // -- Opening book algorithm
    std::pair<int, int> mossa_book = cerca_nel_book();
    if (mossa_book.first != -1)
    {
        profondita_raggiunta = 0;
        tempo_ultima_mossa_ms = 0;
        pezzo_promozione_scelto = 0;
        return mossa_book;
    }

    motore_in_ricerca = true;
    std::memset(mosse_killer, 0, sizeof(mosse_killer));

    scacchiera scacch = b;
    int colore_bot = turno;

    Mossa mosse[256];
    int numero_mosse = genera_mosse(mosse, turno, scacch);

    if (numero_mosse == 0)
    {
        motore_in_ricerca = false;
        return {-1, -1};
    }

    for (int i = 0; i < numero_mosse; i++)
    {
        int vittima = scacch.casa[mosse[i].a];
        int attacc = scacch.casa[mosse[i].da];
        mosse[i].punteggio = (vittima != 0) ? (1000 + vittima_attaccante(attacc, vittima)) : 0;
    }
    std::sort(mosse, mosse + numero_mosse, [](const Mossa &x, const Mossa &y)
              { return x.punteggio > y.punteggio; });

    auto inizio = std::chrono::high_resolution_clock::now();
    int mosse_giocate = (int)storia_mosse.size();
    int limite_ms = 4000; // 5.0 secondi fissi per ogni mossa
    int migliore_idx = 0;
    int profondita_corr = 1;
    const int max_prof = 10;

    nodes_searched = 0;

    int finestra = 50;
    int valore_prec = -INFINITO;

    // -- Iterative deepening algorithm
    while (profondita_corr <= max_prof)
    {
        int alfa_asp, beta_asp;

        // -- Aspiration window algorithm
        if (profondita_corr >= 4 && valore_prec > -INFINITO / 2)
        {
            alfa_asp = valore_prec - finestra;
            beta_asp = valore_prec + finestra;
        }
        else
        {
            alfa_asp = -INFINITO;
            beta_asp = INFINITO;
        }

        int miglior_val = -INFINITO;
        int idx_temp = 0;
        int punteggi_radice[256] = {}; // azzerato ad ogni depth — valori sempre consistenti

        bool ri_cerca = true;
        while (ri_cerca)
        {
            miglior_val = -INFINITO;
            idx_temp = 0;
            ri_cerca = false;

            for (int i = 0; i < numero_mosse; i++)
            {
                int turno_prima_mossa = turno;
                bool matto_bot = false;
                esegui_mossa(scacch, mosse[i].da, mosse[i].a, matto_bot);
                if (mosse[i].promozione != 0 && promozione)
                {
                    scacch.casa[idx_promozione] = mosse[i].promozione | turno_prima_mossa;
                    promozione = false;
                    turno = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
                    avversario = (avversario == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
                }
                punteggi_radice[i] = -minimax(scacch, profondita_corr - 1, -beta_asp, -alfa_asp, colore_bot, 1);
                annulla_mossa(scacch);
                if (punteggi_radice[i] > miglior_val)
                {
                    miglior_val = punteggi_radice[i];
                    idx_temp = i;
                }
            }

            if (miglior_val <= alfa_asp)
            {
                alfa_asp = -INFINITO;
                ri_cerca = true;
            }
            else if (miglior_val >= beta_asp)
            {
                beta_asp = INFINITO;
                ri_cerca = true;
            }
        }

        {
            int temp = (colore_bot == pezzi::bianco) ? temperamento_bianco : temperamento_nero;

            // Se la mossa migliore è un matto, NON applicare il temperamento:
            // scegli sempre il matto più veloce (punteggio più alto tra i matti).
            // MATE_SCORE = 30000, i matti hanno punteggio >= 30000 - PROFONDITA_MASSIMA
            bool miglior_e_matto = (miglior_val >= 30000 - PROFONDITA_MASSIMA);
            if (miglior_e_matto)
            {
                // idx_temp è già la mossa con punteggio più alto — è il matto più veloce.
                // Non fare nulla: mantieni idx_temp così com'è.
            }
            else if (temp > 0)
            {
                // Soglia: accetta solo mosse entro temp cp dalla migliore
                int soglia = miglior_val - temp;

                // Raccoglie i candidati (mosse quasi equivalenti), escludendo i matti
                // dell'avversario (punteggi molto negativi) — già filtrati dalla soglia.
                int candidati[256];
                int n_candidati = 0;
                for (int i = 0; i < numero_mosse; i++)
                    if (punteggi_radice[i] >= soglia)
                        candidati[n_candidati++] = i;

                // Sceglie casualmente tra i candidati
                idx_temp = candidati[rand() % n_candidati];
            }
        }

        valore_prec = miglior_val;
        migliore_idx = idx_temp;
        std::swap(mosse[0], mosse[migliore_idx]);
        migliore_idx = 0;

        auto ora = std::chrono::high_resolution_clock::now();
        auto trascorso = std::chrono::duration_cast<std::chrono::milliseconds>(ora - inizio);
        if (trascorso.count() > 0)
            nodes_per_second = (int)(nodes_searched * 1000LL / trascorso.count());
        if (trascorso.count() >= limite_ms)
            break;
        profondita_corr++;
    }

    auto fine = std::chrono::high_resolution_clock::now();
    auto trascorso = std::chrono::duration_cast<std::chrono::milliseconds>(fine - inizio);

    profondita_raggiunta = profondita_corr;
    tempo_ultima_mossa_ms = (int)trascorso.count();

    pezzo_promozione_scelto = mosse[migliore_idx].promozione;
    motore_in_ricerca = false;
    return {mosse[migliore_idx].da, mosse[migliore_idx].a};
}
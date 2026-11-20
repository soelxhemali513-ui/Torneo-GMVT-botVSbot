#include "scacchiera.h"
#include "raylib.h"
#include "variabili.h"
#include "pezzi.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
using namespace std;

// Stack separato usato dal motore durante la ricerca:
// NON tocca mai storia_mosse globale, eliminando il leak tra partite.
static std::vector<StatoMossa> stack_motore;

scacchiera::scacchiera()
{
    for (int i = 0; i < 64; i++)
        casa[i] = pezzi::vuoto;
    for (int i = 0; i < 12; i++)
        bitboard_pieces[i] = 0ULL;
    occupancies[0] = occupancies[1] = occupancies[2] = 0ULL;
}

int scacchiera::pieceIndex(int p)
{
    if (p == pezzi::vuoto)
        return -1;
    int type = p & 7;
    int color = ((p & 24) == pezzi::nero) ? 1 : 0;
    return color * 6 + (type - 1);
}

void scacchiera::updateBitboards()
{
    for (int i = 0; i < 12; i++)
        bitboard_pieces[i] = 0ULL;
    occupancies[0] = occupancies[1] = occupancies[2] = 0ULL;
    for (int i = 0; i < 64; i++)
    {
        int p = casa[i];
        int idx = pieceIndex(p);
        if (idx >= 0)
        {
            bitboard_pieces[idx] |= (1ULL << i);
        }
    }
    for (int i = 0; i < 6; i++)
        occupancies[0] |= bitboard_pieces[i];
    for (int i = 6; i < 12; i++)
        occupancies[1] |= bitboard_pieces[i];
    occupancies[2] = occupancies[0] | occupancies[1];
}
void fen()
{
    bool colore = false;
    bool spazio = false;
    int casella = 0;
    for (int i = 0; i < stringa_fen.length(); i++)
    {
        if (casella < 64)
        {
            switch (stringa_fen[i])
            {
            case 'K':
                b.casa[casella] = pezzi::re | pezzi::bianco;
                casella++;
                break;
            case 'Q':
                b.casa[casella] = pezzi::regina | pezzi::bianco;
                casella++;
                break;
            case 'R':
                b.casa[casella] = pezzi::torre | pezzi::bianco;
                casella++;
                break;
            case 'B':
                b.casa[casella] = pezzi::alfiere | pezzi::bianco;
                casella++;
                break;
            case 'N':
                b.casa[casella] = pezzi::cavallo | pezzi::bianco;
                casella++;
                break;
            case 'P':
                b.casa[casella] = pezzi::pedone | pezzi::bianco;
                casella++;
                break;
            case 'k':
                b.casa[casella] = pezzi::re | pezzi::nero;
                casella++;
                break;
            case 'q':
                b.casa[casella] = pezzi::regina | pezzi::nero;
                casella++;
                break;
            case 'r':
                b.casa[casella] = pezzi::torre | pezzi::nero;
                casella++;
                break;
            case 'b':
                b.casa[casella] = pezzi::alfiere | pezzi::nero;
                casella++;
                break;
            case 'n':
                b.casa[casella] = pezzi::cavallo | pezzi::nero;
                casella++;
                break;
            case 'p':
                b.casa[casella] = pezzi::pedone | pezzi::nero;
                casella++;
                break;
            case '1':
                b.casa[casella] = pezzi::vuoto;
                casella++;
                break;
            case '2':
                for (int i = 0; i < 2; i++)
                {
                    b.casa[casella] = pezzi::vuoto;
                    casella++;
                }
                break;
            case '3':
                for (int i = 0; i < 3; i++)
                {
                    b.casa[casella] = pezzi::vuoto;
                    casella++;
                }
                break;
            case '4':
                for (int i = 0; i < 4; i++)
                {
                    b.casa[casella] = pezzi::vuoto;
                    casella++;
                }
                break;
            case '5':
                for (int i = 0; i < 5; i++)
                {
                    b.casa[casella] = pezzi::vuoto;
                    casella++;
                }
                break;
            case '6':
                for (int i = 0; i < 6; i++)
                {
                    b.casa[casella] = pezzi::vuoto;
                    casella++;
                }
                break;
            case '7':
                for (int i = 0; i < 7; i++)
                {
                    b.casa[casella] = pezzi::vuoto;
                    casella++;
                }
                break;
            case '8':
                for (int i = 0; i < 8; i++)
                {
                    b.casa[casella] = pezzi::vuoto;
                    casella++;
                }
                break;
            }
        }
        if (stringa_fen[i] == ' ' && !spazio)
        {
            spazio = true;
        }
        if (spazio)
        {
            if (colore == false)
            {
                switch (stringa_fen[i])
                {
                case 'w':
                    turno = pezzi::bianco;
                    colore = true;
                    break;
                case 'b':
                    turno = pezzi::nero;
                    colore = true;
                    break;
                }
            }
            if (colore == true)
            {
                switch (stringa_fen[i])
                {
                case 'K':
                    torre_h1 = true;
                    re_e1 = true;
                    break;
                case 'Q':
                    torre_a1 = true;
                    re_e1 = true;
                    break;
                case 'k':
                    torre_h8 = true;
                    re_e8 = true;
                    break;
                case 'q':
                    torre_a8 = true;
                    re_e8 = true;
                    break;
                case '-':
                    break;
                }
                if (stringa_fen[i] >= 'a' && stringa_fen[i] <= 'h')
                {
                    idx_enpassant = notazione_idx(i);
                    i += 2;
                }
            }
        }
        b.updateBitboards();
    }
}
// manca mosse da fine partita

void tutte_mosse(scacchiera &b, int &riga, int &col, int idx, int &pezzo, int mossa_possibile[], int &mosse_possibili, int &turno, int &avversario, bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8, bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8, int casa_re)
{
    mosse_possibili = 0;
    int mossa_possibile_pin[64];
    int mosse_possibili_pin = 0;
    int nuova_riga, nuova_colonna;
    for (int i = 0; i < 64; i++)
    {
        mossa_possibile_pin[i] = -1;
    }
    // mosse legali pedone
    if (pezzo == (pezzi::pedone | turno))
    {
        int direzione;
        int riga_iniziale;
        int riga_enpassant;
        int riga_finale;
        if (turno == pezzi::bianco)
        {
            direzione = -8;
            riga_iniziale = 6;
            riga_enpassant = 3;
            riga_finale = 0;
        }
        else
        {
            direzione = 8;
            riga_iniziale = 1;
            riga_finale = 7;
            riga_enpassant = 4;
        }
        bool bloccato = true;
        // mossa avanti -- bounds check PRIMA dell'accesso all'array
        {
            int dest_avanti = idx + direzione;
            if (dest_avanti >= 0 && dest_avanti < 64 && !(b.casa[dest_avanti] & turno) && !(b.casa[dest_avanti] & avversario))
            {
                mossa_possibile_pin[mosse_possibili_pin++] = dest_avanti;
                bloccato = false;
            }
        }
        // cattura a sinistra
        {
            int dest_sx = idx + direzione - 1;
            if (col != 0 && dest_sx >= 0 && dest_sx < 64 && b.casa[dest_sx] != pezzi::vuoto && (b.casa[dest_sx] & avversario))
            {
                mossa_possibile_pin[mosse_possibili_pin++] = dest_sx;
            }
        }
        // cattura a destra
        {
            int dest_dx = idx + direzione + 1;
            if (col != 7 && dest_dx >= 0 && dest_dx < 64 && b.casa[dest_dx] != pezzi::vuoto && (b.casa[dest_dx] & avversario))
            {
                mossa_possibile_pin[mosse_possibili_pin++] = dest_dx;
            }
        }
        // mossa avanti di 2
        {
            int dest_doppio = idx + 2 * direzione;
            if (!bloccato && riga == riga_iniziale && dest_doppio >= 0 && dest_doppio < 64 && !(b.casa[dest_doppio] & turno) && !(b.casa[dest_doppio] & avversario))
            {
                mossa_possibile_pin[mosse_possibili_pin++] = dest_doppio;
            }
        }
        // en passant
        if (idx_enpassant != -1)
        {
            // pedone bianco
            if (turno == pezzi::bianco)
            {
                if (riga == 3)
                {
                    if (col > 0 && idx - 9 == idx_enpassant)
                    {
                        mossa_possibile_pin[mosse_possibili_pin++] = idx_enpassant;
                    }
                    if (col < 7 && idx - 7 == idx_enpassant)
                    {
                        mossa_possibile_pin[mosse_possibili_pin++] = idx_enpassant;
                    }
                }
            }
            // pedone nero
            else
            {
                if (riga == 4)
                {
                    if (col > 0 && idx + 7 == idx_enpassant)
                    {
                        mossa_possibile_pin[mosse_possibili_pin++] = idx_enpassant;
                    }
                    if (col < 7 && idx + 9 == idx_enpassant)
                    {
                        mossa_possibile_pin[mosse_possibili_pin++] = idx_enpassant;
                    }
                }
            }
        }
    }

    // mosse legali cavallo
    if (pezzo == (pezzi::cavallo | turno))
    {
        int differenza_riga[8] = {2, 2, 1, 1, -1, -1, -2, -2};
        int differenza_colonna[8] = {1, -1, 2, -2, 2, -2, 1, -1};

        for (int i = 0; i < 8; i++)
        {
            nuova_colonna = col + differenza_colonna[i];
            nuova_riga = riga + differenza_riga[i];

            // controlli i limiti
            if (nuova_colonna >= 0 && nuova_colonna < 8 &&
                nuova_riga >= 0 && nuova_riga < 8)
            {
                int idxC = nuova_riga * 8 + nuova_colonna;

                // accedi all'array
                if (!(b.casa[idxC] & turno))
                {
                    mossa_possibile_pin[mosse_possibili_pin] = idxC;
                    mosse_possibili_pin++;
                }
            }
        }
    }

    // mosse legali alfiere
    if (pezzo == (pezzi::alfiere | turno))
    {
        int differenza_riga[4] = {1, 1, -1, -1};
        int differenza_colonna[4] = {1, -1, 1, -1};

        for (int i = 0; i < 4; i++)
        {
            for (int c = 1; c < 8; c++)
            {
                nuova_colonna = col + differenza_colonna[i] * c;
                nuova_riga = riga + differenza_riga[i] * c;

                // controlli se sei dentro la scacchiera
                if (nuova_colonna < 0 || nuova_colonna >= 8 ||
                    nuova_riga < 0 || nuova_riga >= 8)
                {
                    break;
                }

                int idxA = nuova_riga * 8 + nuova_colonna;

                // se c'è un tuo pezzo blocca
                if (b.casa[idxA] & turno)
                {
                    break;
                }

                // mossa valida
                mossa_possibile_pin[mosse_possibili_pin++] = idxA;

                // se c'è un pezzo avversario puoi catturare ma poi ti fermi
                if (b.casa[idxA] & avversario)
                {
                    break;
                }
            }
        }
    }

    // mosse legali torre
    if (pezzo == (pezzi::torre | turno))
    {
        int differenza_riga[4] = {1, -1, 0, 0};
        int differenza_colonna[4] = {0, 0, 1, -1};

        for (int i = 0; i < 4; i++)
        {
            for (int c = 1; c < 8; c++)
            {
                nuova_colonna = col + differenza_colonna[i] * c;
                nuova_riga = riga + differenza_riga[i] * c;

                // controllo limiti
                if (nuova_colonna < 0 || nuova_colonna >= 8 ||
                    nuova_riga < 0 || nuova_riga >= 8)
                {
                    break;
                }

                int idxT = nuova_riga * 8 + nuova_colonna;

                // se c'è un tuo pezzo blocca
                if (b.casa[idxT] & turno)
                {
                    break;
                }

                // mossa valida
                mossa_possibile_pin[mosse_possibili_pin++] = idxT;

                // se c'è un pezzo avversario cattura e stop
                if (b.casa[idxT] & avversario)
                {
                    break;
                }
            }
        }
    }

    // mosse legali regina
    if (pezzo == (pezzi::regina | turno))
    {
        // =parte diagonale
        int diff_riga_diag[4] = {1, 1, -1, -1};
        int diff_col_diag[4] = {1, -1, 1, -1};

        for (int i = 0; i < 4; i++)
        {
            for (int c = 1; c < 8; c++)
            {
                nuova_colonna = col + diff_col_diag[i] * c;
                nuova_riga = riga + diff_riga_diag[i] * c;

                // controllo limiti
                if (nuova_colonna < 0 || nuova_colonna >= 8 ||
                    nuova_riga < 0 || nuova_riga >= 8)
                    break;

                int idx = nuova_riga * 8 + nuova_colonna;

                // tuo pezzo blocca
                if (b.casa[idx] & turno)
                    break;

                // mossa valida
                mossa_possibile_pin[mosse_possibili_pin++] = idx;

                // pezzo avversario cattura e stop
                if (b.casa[idx] & avversario)
                    break;
            }
        }

        // parte dritta
        int diff_riga_dir[4] = {1, -1, 0, 0};
        int diff_col_dir[4] = {0, 0, 1, -1};

        for (int i = 0; i < 4; i++)
        {
            for (int c = 1; c < 8; c++)
            {
                nuova_colonna = col + diff_col_dir[i] * c;
                nuova_riga = riga + diff_riga_dir[i] * c;

                //  controllo limiti
                if (nuova_colonna < 0 || nuova_colonna >= 8 ||
                    nuova_riga < 0 || nuova_riga >= 8)
                    break;

                int idx = nuova_riga * 8 + nuova_colonna;

                // tuo pezzo blocca
                if (b.casa[idx] & turno)
                    break;

                // mossa valida
                mossa_possibile_pin[mosse_possibili_pin++] = idx;

                // pezzo avversario cattura e stop
                if (b.casa[idx] & avversario)
                    break;
            }
        }
    }

    // mosse legali re
    if (pezzo == (pezzi::re | turno))
    {
        int differenza_riga[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
        int differenza_colonna[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

        for (int i = 0; i < 8; i++)
        {
            nuova_colonna = col + differenza_colonna[i];
            nuova_riga = riga + differenza_riga[i];

            // controllo limiti
            if (nuova_colonna < 0 || nuova_colonna >= 8 ||
                nuova_riga < 0 || nuova_riga >= 8)
                continue;

            int idxR = nuova_riga * 8 + nuova_colonna;

            // se non è un tuo pezzo mossa valida
            if (!(b.casa[idxR] & turno))
            {
                mossa_possibile_pin[mosse_possibili_pin++] = idxR;
            }
        }

        //  ARROCCO
        if (pezzo == (pezzi::re | pezzi::bianco))
        {
            // corto
            if (re_e1 && torre_h1 &&
                b.casa[61] == pezzi::vuoto &&
                b.casa[62] == pezzi::vuoto)
            {
                mossa_possibile_pin[mosse_possibili_pin++] = 62;
            }

            // lungo
            if (re_e1 && torre_a1 &&
                b.casa[57] == pezzi::vuoto &&
                b.casa[58] == pezzi::vuoto &&
                b.casa[59] == pezzi::vuoto)
            {
                mossa_possibile_pin[mosse_possibili_pin++] = 58;
            }
        }
        else if (pezzo == (pezzi::re | pezzi::nero))
        {
            // corto
            if (re_e8 && torre_h8 &&
                b.casa[5] == pezzi::vuoto &&
                b.casa[6] == pezzi::vuoto)
            {
                mossa_possibile_pin[mosse_possibili_pin++] = 6;
            }

            // lungo
            if (re_e8 && torre_a8 &&
                b.casa[1] == pezzi::vuoto &&
                b.casa[2] == pezzi::vuoto &&
                b.casa[3] == pezzi::vuoto)
            {
                mossa_possibile_pin[mosse_possibili_pin++] = 2;
            }
        }
    }
    for (int i = 0; i < mosse_possibili_pin; i++)
    {
        if (mossa_possibile_pin[i] != -1)
            mossa_possibile[mosse_possibili++] = mossa_possibile_pin[i];
    }
}
void mosse_pseudoLegali(scacchiera &b, int &riga, int &col, int idx, int &pezzo, int mossa_possibile[], int &mosse_possibili, int &turno, int &avversario, bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8, bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8, int casa_re)
{
    mosse_possibili = 0;
    int mossa_possibile_pin[64];
    int mosse_possibili_pin = 0;
    int nuova_riga, nuova_colonna;
    for (int i = 0; i < 64; i++)
    {
        mossa_possibile_pin[i] = -1;
    }
    // mosse legali pedone
    if (pezzo == (pezzi::pedone | turno))
    {
        int direzione;
        int riga_iniziale;
        int riga_enpassant;
        int riga_finale;
        if (turno == pezzi::bianco)
        {
            direzione = -8;
            riga_iniziale = 6;
            riga_enpassant = 3;
            riga_finale = 0;
        }
        else
        {
            direzione = 8;
            riga_iniziale = 1;
            riga_finale = 7;
            riga_enpassant = 4;
        }
        bool bloccato = true;
        // mossa avanti -- bounds check PRIMA dell'accesso all'array
        {
            int dest_avanti = idx + direzione;
            if (dest_avanti >= 0 && dest_avanti < 64 && !(b.casa[dest_avanti] & turno) && !(b.casa[dest_avanti] & avversario))
            {
                mossa_possibile_pin[mosse_possibili_pin++] = dest_avanti;
                bloccato = false;
            }
        }
        // cattura a sinistra
        {
            int dest_sx = idx + direzione - 1;
            if (col != 0 && dest_sx >= 0 && dest_sx < 64 && b.casa[dest_sx] != pezzi::vuoto && (b.casa[dest_sx] & avversario))
            {
                mossa_possibile_pin[mosse_possibili_pin++] = dest_sx;
            }
        }
        // cattura a destra
        {
            int dest_dx = idx + direzione + 1;
            if (col != 7 && dest_dx >= 0 && dest_dx < 64 && b.casa[dest_dx] != pezzi::vuoto && (b.casa[dest_dx] & avversario))
            {
                mossa_possibile_pin[mosse_possibili_pin++] = dest_dx;
            }
        }
        // mossa avanti di 2
        {
            int dest_doppio = idx + 2 * direzione;
            if (!bloccato && riga == riga_iniziale && dest_doppio >= 0 && dest_doppio < 64 && !(b.casa[dest_doppio] & turno) && !(b.casa[dest_doppio] & avversario))
            {
                mossa_possibile_pin[mosse_possibili_pin++] = dest_doppio;
            }
        }
        // en passant
        if (idx_enpassant != -1)
        {
            // pedone bianco
            if (turno == pezzi::bianco)
            {
                if (riga == 3)
                {
                    if (col > 0 && idx - 9 == idx_enpassant)
                    {
                        mossa_possibile_pin[mosse_possibili_pin++] = idx_enpassant;
                    }
                    if (col < 7 && idx - 7 == idx_enpassant)
                    {
                        mossa_possibile_pin[mosse_possibili_pin++] = idx_enpassant;
                    }
                }
            }
            // pedone nero
            else
            {
                if (riga == 4)
                {
                    if (col > 0 && idx + 7 == idx_enpassant)
                    {
                        mossa_possibile_pin[mosse_possibili_pin++] = idx_enpassant;
                    }
                    if (col < 7 && idx + 9 == idx_enpassant)
                    {
                        mossa_possibile_pin[mosse_possibili_pin++] = idx_enpassant;
                    }
                }
            }
        }
    }

    // mosse legali cavallo
    if (pezzo == (pezzi::cavallo | turno))
    {
        int differenza_riga[8] = {2, 2, 1, 1, -1, -1, -2, -2};
        int differenza_colonna[8] = {1, -1, 2, -2, 2, -2, 1, -1};

        for (int i = 0; i < 8; i++)
        {
            nuova_colonna = col + differenza_colonna[i];
            nuova_riga = riga + differenza_riga[i];

            // controlli i limiti
            if (nuova_colonna >= 0 && nuova_colonna < 8 &&
                nuova_riga >= 0 && nuova_riga < 8)
            {
                int idxC = nuova_riga * 8 + nuova_colonna;

                // accedi all'array
                if (!(b.casa[idxC] & turno))
                {
                    mossa_possibile_pin[mosse_possibili_pin] = idxC;
                    mosse_possibili_pin++;
                }
            }
        }
    }

    // mosse legali alfiere
    if (pezzo == (pezzi::alfiere | turno))
    {
        int differenza_riga[4] = {1, 1, -1, -1};
        int differenza_colonna[4] = {1, -1, 1, -1};

        for (int i = 0; i < 4; i++)
        {
            for (int c = 1; c < 8; c++)
            {
                nuova_colonna = col + differenza_colonna[i] * c;
                nuova_riga = riga + differenza_riga[i] * c;

                // controlli se sei dentro la scacchiera
                if (nuova_colonna < 0 || nuova_colonna >= 8 ||
                    nuova_riga < 0 || nuova_riga >= 8)
                {
                    break;
                }

                int idxA = nuova_riga * 8 + nuova_colonna;

                // se c'è un tuo pezzo blocca
                if (b.casa[idxA] & turno)
                {
                    break;
                }

                // mossa valida
                mossa_possibile_pin[mosse_possibili_pin++] = idxA;

                // se c'è un pezzo avversario puoi catturare ma poi ti fermi
                if (b.casa[idxA] & avversario)
                {
                    break;
                }
            }
        }
    }

    // mosse legali torre
    if (pezzo == (pezzi::torre | turno))
    {
        int differenza_riga[4] = {1, -1, 0, 0};
        int differenza_colonna[4] = {0, 0, 1, -1};

        for (int i = 0; i < 4; i++)
        {
            for (int c = 1; c < 8; c++)
            {
                nuova_colonna = col + differenza_colonna[i] * c;
                nuova_riga = riga + differenza_riga[i] * c;

                // controllo limiti
                if (nuova_colonna < 0 || nuova_colonna >= 8 ||
                    nuova_riga < 0 || nuova_riga >= 8)
                {
                    break;
                }

                int idxT = nuova_riga * 8 + nuova_colonna;

                // se c'è un tuo pezzo blocca
                if (b.casa[idxT] & turno)
                {
                    break;
                }

                // mossa valida
                mossa_possibile_pin[mosse_possibili_pin++] = idxT;

                // se c'è un pezzo avversario cattura e stop
                if (b.casa[idxT] & avversario)
                {
                    break;
                }
            }
        }
    }

    // mosse legali regina
    if (pezzo == (pezzi::regina | turno))
    {
        // =parte diagonale
        int diff_riga_diag[4] = {1, 1, -1, -1};
        int diff_col_diag[4] = {1, -1, 1, -1};

        for (int i = 0; i < 4; i++)
        {
            for (int c = 1; c < 8; c++)
            {
                nuova_colonna = col + diff_col_diag[i] * c;
                nuova_riga = riga + diff_riga_diag[i] * c;

                // controllo limiti
                if (nuova_colonna < 0 || nuova_colonna >= 8 ||
                    nuova_riga < 0 || nuova_riga >= 8)
                    break;

                int idx = nuova_riga * 8 + nuova_colonna;

                // tuo pezzo blocca
                if (b.casa[idx] & turno)
                    break;

                // mossa valida
                mossa_possibile_pin[mosse_possibili_pin++] = idx;

                // pezzo avversario cattura e stop
                if (b.casa[idx] & avversario)
                    break;
            }
        }

        // parte dritta
        int diff_riga_dir[4] = {1, -1, 0, 0};
        int diff_col_dir[4] = {0, 0, 1, -1};

        for (int i = 0; i < 4; i++)
        {
            for (int c = 1; c < 8; c++)
            {
                nuova_colonna = col + diff_col_dir[i] * c;
                nuova_riga = riga + diff_riga_dir[i] * c;

                //  controllo limiti
                if (nuova_colonna < 0 || nuova_colonna >= 8 ||
                    nuova_riga < 0 || nuova_riga >= 8)
                    break;

                int idx = nuova_riga * 8 + nuova_colonna;

                // tuo pezzo blocca
                if (b.casa[idx] & turno)
                    break;

                // mossa valida
                mossa_possibile_pin[mosse_possibili_pin++] = idx;

                // pezzo avversario cattura e stop
                if (b.casa[idx] & avversario)
                    break;
            }
        }
    }

    // mosse legali re
    if (pezzo == (pezzi::re | turno))
    {
        int differenza_riga[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
        int differenza_colonna[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

        for (int i = 0; i < 8; i++)
        {
            nuova_colonna = col + differenza_colonna[i];
            nuova_riga = riga + differenza_riga[i];

            // controllo limiti
            if (nuova_colonna < 0 || nuova_colonna >= 8 ||
                nuova_riga < 0 || nuova_riga >= 8)
                continue;

            int idxR = nuova_riga * 8 + nuova_colonna;

            // se non è un tuo pezzo mossa valida
            if (!(b.casa[idxR] & turno))
            {
                mossa_possibile_pin[mosse_possibili_pin++] = idxR;
            }
        }

        //  ARROCCO

        if (!scacco_al_re(b, turno, avversario, casa_re, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8))
        {
            if (pezzo == (pezzi::re | pezzi::bianco))
            {
                // corto
                if (re_e1 && torre_h1 &&
                    b.casa[61] == pezzi::vuoto &&
                    b.casa[62] == pezzi::vuoto &&
                    !arrocco_si(b, turno, avversario, 61, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8))
                {
                    mossa_possibile_pin[mosse_possibili_pin++] = 62;
                }

                // lungo
                if (re_e1 && torre_a1 &&
                    b.casa[57] == pezzi::vuoto &&
                    b.casa[58] == pezzi::vuoto &&
                    b.casa[59] == pezzi::vuoto &&
                    !arrocco_si(b, turno, avversario, 59, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8))
                {
                    mossa_possibile_pin[mosse_possibili_pin++] = 58;
                }
            }
            else if (pezzo == (pezzi::re | pezzi::nero))
            {
                // corto
                if (re_e8 && torre_h8 &&
                    b.casa[5] == pezzi::vuoto &&
                    b.casa[6] == pezzi::vuoto &&
                    !arrocco_si(b, turno, avversario, 5, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8))
                {
                    mossa_possibile_pin[mosse_possibili_pin++] = 6;
                }

                // lungo
                if (re_e8 && torre_a8 &&
                    b.casa[1] == pezzi::vuoto &&
                    b.casa[2] == pezzi::vuoto &&
                    b.casa[3] == pezzi::vuoto &&
                    !arrocco_si(b, turno, avversario, 3, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8))
                {
                    mossa_possibile_pin[mosse_possibili_pin++] = 2;
                }
            }
        }
    }
    for (int i = 0; i < 64; i++)
    {
        if (b.casa[i] == (pezzi::re | turno))
        {
            casa_re = i;
            break;
        }
    }
    for (int i = 0; i < mosse_possibili_pin; i++)
    {
        // Usa sempre esegui_mossa_bot per la validazione delle mosse pseudolegali:
        // evita suoni, aggiornamenti UI e scrittura di mossaC/R durante il filtraggio.
        esegui_mossa_bot(b, idx, mossa_possibile_pin[i], matto);
        int nuova_casa_re = casa_re;
        if (pezzo == (pezzi::re | turno))
            nuova_casa_re = mossa_possibile_pin[i];

        if (!scacco_al_re(b, turno, avversario, nuova_casa_re, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8))
        {
            mossa_possibile[mosse_possibili++] = mossa_possibile_pin[i];
        }
        annulla_mossa(b);
    }
}
static bool casa_attaccata_da(const scacchiera &b, int casella, int colore_attaccante)
{
    int riga = casella / 8;
    int col = casella % 8;

    if (colore_attaccante == pezzi::bianco)
    {
        if (riga < 7 && col > 0 && b.casa[casella + 7] == (pezzi::pedone | pezzi::bianco))
            return true;
        if (riga < 7 && col < 7 && b.casa[casella + 9] == (pezzi::pedone | pezzi::bianco))
            return true;
    }
    else
    {
        if (riga > 0 && col > 0 && b.casa[casella - 9] == (pezzi::pedone | pezzi::nero))
            return true;
        if (riga > 0 && col < 7 && b.casa[casella - 7] == (pezzi::pedone | pezzi::nero))
            return true;
    }

    static const int dr_cav[8] = {-2, -2, -1, -1, 1, 1, 2, 2};
    static const int dc_cav[8] = {-1, 1, -2, 2, -2, 2, -1, 1};
    for (int i = 0; i < 8; i++)
    {
        int rr = riga + dr_cav[i];
        int cc = col + dc_cav[i];
        if (rr < 0 || rr > 7 || cc < 0 || cc > 7)
            continue;
        int p = b.casa[rr * 8 + cc];
        if (p == (pezzi::cavallo | colore_attaccante))
            return true;
    }

    static const int dr_re[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    static const int dc_re[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    for (int i = 0; i < 8; i++)
    {
        int rr = riga + dr_re[i];
        int cc = col + dc_re[i];
        if (rr < 0 || rr > 7 || cc < 0 || cc > 7)
            continue;
        int p = b.casa[rr * 8 + cc];
        if (p == (pezzi::re | colore_attaccante))
            return true;
    }

    static const int dr_alfiere[4] = {-1, -1, 1, 1};
    static const int dc_alfiere[4] = {-1, 1, -1, 1};
    for (int d = 0; d < 4; d++)
    {
        int rr = riga + dr_alfiere[d], cc = col + dc_alfiere[d];
        while (rr >= 0 && rr < 8 && cc >= 0 && cc < 8)
        {
            int p = b.casa[rr * 8 + cc];
            if (p != pezzi::vuoto)
            {
                int tipo = p & 7;
                int colore = p & 24;
                if (colore == colore_attaccante && (tipo == pezzi::alfiere || tipo == pezzi::regina))
                    return true;
                break;
            }
            rr += dr_alfiere[d];
            cc += dc_alfiere[d];
        }
    }

    static const int dr_torre[4] = {-1, 1, 0, 0};
    static const int dc_torre[4] = {0, 0, -1, 1};
    for (int d = 0; d < 4; d++)
    {
        int rr = riga + dr_torre[d], cc = col + dc_torre[d];
        while (rr >= 0 && rr < 8 && cc >= 0 && cc < 8)
        {
            int p = b.casa[rr * 8 + cc];
            if (p != pezzi::vuoto)
            {
                int tipo = p & 7;
                int colore = p & 24;
                if (colore == colore_attaccante && (tipo == pezzi::torre || tipo == pezzi::regina))
                    return true;
                break;
            }
            rr += dr_torre[d];
            cc += dc_torre[d];
        }
    }
    return false;
}

bool scacco_al_re(const scacchiera &b, int turno, int avversario, int &casa_re, bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8, bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8)
{
    (void)enpassant;
    (void)idx_enpassant;
    (void)re_e1;
    (void)re_e8;
    (void)torre_a1;
    (void)torre_a8;
    (void)torre_h1;
    (void)torre_h8;
    for (int i = 0; i < 64; i++)
    {
        if (b.casa[i] == (pezzi::re | turno))
        {
            casa_re = i;
            break;
        }
    }
    return casa_attaccata_da(b, casa_re, avversario);
}
void esci_da_scacco(scacchiera b, int &casa_re, int avversario, int turno, int mossa_possibile[], int &mosse_possibili, int riga, int col, int idx, int pezzo, bool &matto, bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8, bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8)
{
    int mossa_possibile_scacco[27];
    for (int i = 0; i < 27; i++)
    {
        mossa_possibile_scacco[i] = -1;
    }
    mosse_possibili = 0;
    int mosse_possibili_scacco = 0;
    mosse_pseudoLegali(b, riga, col, idx, pezzo, mossa_possibile_scacco, mosse_possibili_scacco, turno, avversario, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8, casa_re);
    for (int i = 0; i < mosse_possibili_scacco; i++)
    {
        scacchiera board = b;
        board.casa[mossa_possibile_scacco[i]] = pezzo;
        board.casa[idx] = pezzi::vuoto;
        int nuova_casa_re = casa_re;
        if (pezzo == (pezzi::re | turno))
            nuova_casa_re = mossa_possibile_scacco[i];

        if (!scacco_al_re(board, turno, avversario, nuova_casa_re, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8))
        {
            mossa_possibile[mosse_possibili++] = mossa_possibile_scacco[i];
        }
    }
}
void mosse_legali(scacchiera b, int &casa_re, int avversario, int turno, int mossa_possibile[], int &mosse_possibili, int riga, int col, int idx, int pezzo, bool &matto, bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8, bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8)
{
    if (!scacco_al_re(b, turno, avversario, casa_re, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8))
    {
        mosse_pseudoLegali(b, riga, col, idx, pezzo, mossa_possibile, mosse_possibili, turno, avversario, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8, casa_re);
    }
    else
    {
        esci_da_scacco(b, casa_re, avversario, turno, mossa_possibile, mosse_possibili, riga, col, idx, pezzo, matto, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8);
        // setta matto solo in UI, non durante la ricerca del bot
        if (!motore_in_ricerca && mosse_possibili == 0 && scacco_matto(turno, avversario, b))
            matto = true;
    }
}
bool esegui_mossa(scacchiera &b, int idx, int idx2, bool &matto)
{
    if ((b.casa[idx] & 7) != pezzi::pedone || b.casa[idx2] != 0)
    {
        fifty_move++;
    }
    else
    {
        fifty_move = 0;
    }
    int pezzo = b.casa[idx];

    // Salva stato per annulla_mossa
    StatoMossa stato;
    stato.idx = idx;
    stato.idx2 = idx2;
    stato.pezzo = pezzo;
    stato.catturato = b.casa[idx2];
    stato.idx_enpassant_prec = idx_enpassant;
    stato.pedone_ep_idx = -1;
    stato.pedone_ep_valore = pezzi::vuoto;
    stato.torre_da = -1;
    stato.torre_a = -1;
    stato.torre_valore = pezzi::vuoto;
    stato.re_e1_prec = re_e1;
    stato.re_e8_prec = re_e8;
    stato.torre_a1_prec = torre_a1;
    stato.torre_a8_prec = torre_a8;
    stato.torre_h1_prec = torre_h1;
    stato.torre_h8_prec = torre_h8;
    stato.fifty_move_prec = fifty_move;
    stato.turno_prec = turno;
    stato.era_promozione = false;

    // ARROCCO
    if (pezzo == (pezzi::re | turno))
    {
        if (idx == 60 && idx2 == 62)
        {
            stato.torre_da = 63;
            stato.torre_a = 61;
            stato.torre_valore = pezzi::torre | pezzi::bianco;
            b.casa[61] = pezzi::torre | pezzi::bianco;
            b.casa[63] = pezzi::vuoto;
        }
        if (idx == 60 && idx2 == 58)
        {
            stato.torre_da = 56;
            stato.torre_a = 59;
            stato.torre_valore = pezzi::torre | pezzi::bianco;
            b.casa[59] = pezzi::torre | pezzi::bianco;
            b.casa[56] = pezzi::vuoto;
        }
        if (idx == 4 && idx2 == 6)
        {
            stato.torre_da = 7;
            stato.torre_a = 5;
            stato.torre_valore = pezzi::torre | pezzi::nero;
            b.casa[5] = pezzi::torre | pezzi::nero;
            b.casa[7] = pezzi::vuoto;
        }
        if (idx == 4 && idx2 == 2)
        {
            stato.torre_da = 0;
            stato.torre_a = 3;
            stato.torre_valore = pezzi::torre | pezzi::nero;
            b.casa[3] = pezzi::torre | pezzi::nero;
            b.casa[0] = pezzi::vuoto;
        }
    }

    // MODIFICA EN PASSANT
    if (pezzo == (pezzi::pedone | turno))
    {
        // cattura en passant
        if (idx2 == idx_enpassant)
        {
            int pedone_catturato = (turno == pezzi::bianco) ? idx2 + 8 : idx2 - 8;
            stato.pedone_ep_idx = pedone_catturato;
            stato.pedone_ep_valore = b.casa[pedone_catturato];
            b.casa[pedone_catturato] = pezzi::vuoto;
        }

        // aggiorna idx_enpassant solo se il pedone fa doppio passo
        if (abs(idx2 - idx) == 16)
        {
            idx_enpassant = (idx + idx2) / 2;
        }
        else
        {
            idx_enpassant = -1;
        }
    }

    // rileva tipo di mossa per audio
    bool era_cattura = (stato.catturato != pezzi::vuoto) || (stato.pedone_ep_idx != -1);
    bool era_arrocco = (pezzo == (pezzi::re | turno)) && (stato.torre_da != -1);
    int riga2_promo = idx2 / 8;
    bool era_promozione_audio = (pezzo == (pezzi::pedone | turno)) && (riga2_promo == 0 || riga2_promo == 7);

    // muovi pezzo
    b.casa[idx2] = pezzo;
    b.casa[idx] = pezzi::vuoto;

    // se muove il re
    if (pezzo == (pezzi::re | pezzi::bianco))
    {
        re_e1 = false;
        casa_re = idx2; // aggiorna il globale
    }
    if (pezzo == (pezzi::re | pezzi::nero))
    {
        re_e8 = false;
        casa_re = idx2; // aggiorna il globale
    }

    // se muove una torre
    if (pezzo == (pezzi::torre | pezzi::bianco))
    {
        if (idx == 56)
            torre_a1 = false;
        if (idx == 63)
            torre_h1 = false;
    }
    if (pezzo == (pezzi::torre | pezzi::nero))
    {
        if (idx == 0)
            torre_a8 = false;
        if (idx == 7)
            torre_h8 = false;
    }

    if (idx2 == 56)
        torre_a1 = false; // torre bianca a1 catturata
    if (idx2 == 63)
        torre_h1 = false; // torre bianca h1 catturata
    if (idx2 == 0)
        torre_a8 = false; // torre nera  a8 catturata
    if (idx2 == 7)
        torre_h8 = false; // torre nera  h8 catturata

    // promozione
    int riga2 = idx2 / 8;
    if (pezzo == (pezzi::pedone | turno) && (riga2 == 0 || riga2 == 7))
    {
        promozione = true;
        idx_promozione = idx2;
        stato.era_promozione = true;
    }

    if (!motore_in_ricerca)
    {
        mossaC = idx % 8;
        mossaR = idx / 8;
        mossaC_a = idx2 % 8;
        mossaR_a = idx2 / 8;
    }
    // cambio turno
    if (!promozione)
    {
        turno = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
    }
    mossa = false;
    if (!motore_in_ricerca)
    {
        b.updateBitboards();
        uint64_t hashPosizione = calcolaHash(b, turno);
        history.push_back(hashPosizione);

        // riproduci suono (dopo cambio turno per rilevare scacco)
        int avv_audio = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
        bool scacco_audio = scacco_al_re(b, turno, avv_audio, casa_re, enpassant, idx_enpassant,
                                         re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8);
        // Ferma eventuali suoni precedenti ancora in riproduzione
        StopSound(suono_arrocco);
        StopSound(suono_scacco);
        StopSound(suono_promozione);
        StopSound(suono_capture);
        StopSound(suono_move);
        if (era_arrocco)
            PlaySound(suono_arrocco);
        else if (scacco_audio)
            PlaySound(suono_scacco);
        else if (era_promozione_audio)
            PlaySound(suono_promozione);
        else if (era_cattura)
            PlaySound(suono_capture);
        else
            PlaySound(suono_move);
    }

    if (motore_in_ricerca)
        stack_motore.push_back(stato);
    else
        storia_mosse.push_back(stato);

    return true;
}
void annulla_mossa(scacchiera &b)
{
    std::vector<StatoMossa> &stack = motore_in_ricerca ? stack_motore : storia_mosse;

    if (stack.empty())
        return;

    StatoMossa stato = stack.back();
    stack.pop_back();

    // Ripristina il pezzo mosso e la casa di arrivo
    b.casa[stato.idx] = stato.pezzo;
    b.casa[stato.idx2] = stato.catturato;

    // Annulla arrocco: rimetti la torre dove era
    if (stato.torre_da != -1)
    {
        b.casa[stato.torre_da] = stato.torre_valore;
        b.casa[stato.torre_a] = pezzi::vuoto;
    }

    // Annulla en passant: rimetti il pedone catturato
    if (stato.pedone_ep_idx != -1)
        b.casa[stato.pedone_ep_idx] = stato.pedone_ep_valore;

    // Ripristina tutti i flag
    re_e1 = stato.re_e1_prec;
    re_e8 = stato.re_e8_prec;
    torre_a1 = stato.torre_a1_prec;
    torre_a8 = stato.torre_a8_prec;
    torre_h1 = stato.torre_h1_prec;
    torre_h8 = stato.torre_h8_prec;

    idx_enpassant = stato.idx_enpassant_prec;
    fifty_move = stato.fifty_move_prec;
    turno = stato.turno_prec;
    avversario = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;

    // Aggiorna casa_re
    for (int i = 0; i < 64; i++)
        if (b.casa[i] == (pezzi::re | turno))
        {
            casa_re = i;
            break;
        }

    // Annulla promozione in corso
    promozione = false;
    idx_promozione = -1;

    if (!motore_in_ricerca)
    {
        if (!history.empty())
            history.pop_back();
        b.updateBitboards();
        mossa_fatta = false;
        selezione = false;
        mosse_possibili = 0;
        matto = false;
        draw = false;
        mossa = true;
    }
}
bool arrocco_si(const scacchiera &b, int turno, int avversario, int casa_re, bool &enpassant, int &idx_enpassant, bool re_e1, bool re_e8, bool torre_a1, bool torre_a8, bool torre_h1, bool torre_h8)
{
    (void)turno;
    (void)enpassant;
    (void)idx_enpassant;
    (void)re_e1;
    (void)re_e8;
    (void)torre_a1;
    (void)torre_a8;
    (void)torre_h1;
    (void)torre_h8;
    return casa_attaccata_da(b, casa_re, avversario);
}
int notazione_idx(int j)
{
    int riga;
    int colonna;
    string notazione = string(1, stringa_fen[j]) + string(1, stringa_fen[j + 1]);
    colonna = notazione[0] - 'a';
    riga = 8 - (notazione[1] - '0');
    int indice = riga * 8 + colonna;
    return indice;
}
bool scacco_matto(int turno, int avversario, scacchiera board_sm)
{
    if (scacco_al_re(board_sm, turno, avversario, casa_re, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8))
    {
        for (int i = 0; i < 64; i++)
        {
            if (board_sm.casa[i] & turno)
            {
                int riga_i = i / 8;
                int col_i = i % 8;
                int pezzo_i = board_sm.casa[i];
                int mosse_tmp = 0;
                int mossa_tmp[27];
                for (int t = 0; t < 27; t++)
                    mossa_tmp[t] = -1;
                esci_da_scacco(board_sm, casa_re, avversario, turno, mossa_tmp, mosse_tmp, riga_i, col_i, i, pezzo_i, matto, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8);
                if (mosse_tmp > 0)
                    return false;
            }
        }
        return true;
    }
    return false;
}
bool stelmate()
{
    if (scacco_al_re(b, turno, avversario, casa_re, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8))
        return false;
    for (int i = 0; i < 64; i++)
    {
        if (b.casa[i] & turno)
        {
            int riga_i = i / 8;
            int col_i = i % 8;
            int pezzo_i = b.casa[i];
            int mosse_tmp = 0;
            int mossa_tmp[27];
            for (int t = 0; t < 27; t++)
                mossa_tmp[t] = -1;

            mosse_legali(b, casa_re, avversario, turno, mossa_tmp, mosse_tmp, riga_i, col_i, i, pezzo_i, matto, enpassant, idx_enpassant, re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8);

            if (mosse_tmp > 0)
                return false;
        }
    }
    return true;
}
bool materiale_insufficente()
{
    int altri = 0;
    int cavallo = 0;
    int alfiereLight_bianco = 0;
    int alfiereLight_nero = 0;
    int alfiereDark_bianco = 0;
    int alfiereDark_nero = 0;
    int alfiere = 0;
    int cavallo_nero = 0;
    int cavallo_bianco = 0;

    for (int i = 0; i < 64; i++)
    {
        int p = b.casa[i] & 7;
        if (p == pezzi::regina || p == pezzi::pedone || p == pezzi::torre)
        {
            return false;
        }
        if (p == pezzi::alfiere)
        {
            alfiere++;
            if ((b.casa[i] & 24) == pezzi::bianco)
            {
                if ((i / 8 + i % 8) % 2 == 0)
                {
                    alfiereDark_bianco++;
                }
                else
                {
                    alfiereLight_bianco++;
                }
            }
            else
            {
                if ((i / 8 + i % 8) % 2 == 0)
                {
                    alfiereDark_nero++;
                }
                else
                {
                    alfiereLight_nero++;
                }
            }
        }
        if (p == pezzi::cavallo)
        {
            cavallo++;
            if ((b.casa[i] & 24) == pezzi::bianco)
            {
                cavallo_bianco++;
            }
            else
            {
                cavallo_nero++;
            }
        }
    }

    if (alfiere == 0 && cavallo == 0)
    {
        return true;
    }
    if (alfiere == 1 && cavallo == 0)
    {
        return true;
    }
    if (alfiere == 0 && cavallo == 1)
    {
        return true;
    }
    if (alfiereLight_bianco == 1 && alfiereLight_nero == 1 && alfiereDark_bianco == 0 && alfiereDark_nero == 0)
    {
        return true;
    }
    if (alfiereLight_bianco == 0 && alfiereLight_nero == 0 && alfiereDark_bianco == 1 && alfiereDark_nero == 1)
    {
        return true;
    }
    if (cavallo_bianco == 1 && cavallo_nero == 1)
    {
        return true;
    }
    return false;
}
bool pareggio()
{
    if (stelmate() || materiale_insufficente() || ripetizione(b, turno))
    { //|| (fifty_move>=100)
        return true;
    }
    else
    {
        return false;
    }
}
bool ripetizione(scacchiera b, int turno)
{
    uint64_t hashPosizione = calcolaHash(b, turno);
    int count = 0;

    // scorri la history dall'ultima posizione
    for (int i = history.size() - 1; i >= 0; i--)
    {
        if (history[i] == hashPosizione)
            count++;
        if (count == 3) // stessa posizione apparsa 3 volte
            return true;
    }

    return false;
}
void risultato_partita()
{

    if (mossa_fatta && !promozione)
    {
        avversario = (avversario == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
        matto = scacco_matto(turno, avversario, b);
        draw = pareggio();
    }

    mossa_fatta = false;
}

// per la 50 move rule (memorizza le posizioni in un cert modo dentro a un vettore (che è un array senza dimensione massima))
uint64_t calcolaHash(scacchiera &b, int turno)
{
    uint64_t hash = 0;

    for (int i = 0; i < 64; i++)
    {
        int pezzo = b.casa[i];
        if (pezzo != pezzi::vuoto)
        {
            hash ^= ((uint64_t)pezzo << (i % 64));
        }
    }

    // combina anche il turno
    if (turno == pezzi::nero)
    {
        hash ^= 0xF0F0F0F0F0F0F0F0ULL;
    }

    return hash;
}

bool esegui_mossa_bot(scacchiera &b, int idx, int idx2, bool &matto)
{
    if ((b.casa[idx] & 7) != pezzi::pedone || b.casa[idx2] != 0)
    {
        fifty_move++;
    }
    else
    {
        fifty_move = 0;
    }
    int pezzo = b.casa[idx];

    // Salva stato per annulla_mossa
    StatoMossa stato;
    stato.idx = idx;
    stato.idx2 = idx2;
    stato.pezzo = pezzo;
    stato.catturato = b.casa[idx2];
    stato.idx_enpassant_prec = idx_enpassant;
    stato.pedone_ep_idx = -1;
    stato.pedone_ep_valore = pezzi::vuoto;
    stato.torre_da = -1;
    stato.torre_a = -1;
    stato.torre_valore = pezzi::vuoto;
    stato.re_e1_prec = re_e1;
    stato.re_e8_prec = re_e8;
    stato.torre_a1_prec = torre_a1;
    stato.torre_a8_prec = torre_a8;
    stato.torre_h1_prec = torre_h1;
    stato.torre_h8_prec = torre_h8;
    stato.fifty_move_prec = fifty_move;
    stato.turno_prec = turno;
    stato.era_promozione = false;

    // ARROCCO
    if (pezzo == (pezzi::re | turno))
    {
        if (idx == 60 && idx2 == 62)
        {
            stato.torre_da = 63;
            stato.torre_a = 61;
            stato.torre_valore = pezzi::torre | pezzi::bianco;
            b.casa[61] = pezzi::torre | pezzi::bianco;
            b.casa[63] = pezzi::vuoto;
        }
        if (idx == 60 && idx2 == 58)
        {
            stato.torre_da = 56;
            stato.torre_a = 59;
            stato.torre_valore = pezzi::torre | pezzi::bianco;
            b.casa[59] = pezzi::torre | pezzi::bianco;
            b.casa[56] = pezzi::vuoto;
        }
        if (idx == 4 && idx2 == 6)
        {
            stato.torre_da = 7;
            stato.torre_a = 5;
            stato.torre_valore = pezzi::torre | pezzi::nero;
            b.casa[5] = pezzi::torre | pezzi::nero;
            b.casa[7] = pezzi::vuoto;
        }
        if (idx == 4 && idx2 == 2)
        {
            stato.torre_da = 0;
            stato.torre_a = 3;
            stato.torre_valore = pezzi::torre | pezzi::nero;
            b.casa[3] = pezzi::torre | pezzi::nero;
            b.casa[0] = pezzi::vuoto;
        }
    }

    // MODIFICA EN PASSANT
    if (pezzo == (pezzi::pedone | turno))
    {
        // cattura en passant
        if (idx2 == idx_enpassant)
        {
            int pedone_catturato = (turno == pezzi::bianco) ? idx2 + 8 : idx2 - 8;
            stato.pedone_ep_idx = pedone_catturato;
            stato.pedone_ep_valore = b.casa[pedone_catturato];
            b.casa[pedone_catturato] = pezzi::vuoto;
        }

        // aggiorna idx_enpassant solo se il pedone fa doppio passo
        if (abs(idx2 - idx) == 16)
        {
            idx_enpassant = (idx + idx2) / 2;
        }
        else
        {
            idx_enpassant = -1;
        }
    }

    // muovi pezzo
    b.casa[idx2] = pezzo;
    b.casa[idx] = pezzi::vuoto;

    // se muove il re
    if (pezzo == (pezzi::re | pezzi::bianco))
    {
        re_e1 = false;
        casa_re = idx2; // aggiorna il globale
    }
    if (pezzo == (pezzi::re | pezzi::nero))
    {
        re_e8 = false;
        casa_re = idx2; // aggiorna il globale
    }

    // se muove una torre
    if (pezzo == (pezzi::torre | pezzi::bianco))
    {
        if (idx == 56)
            torre_a1 = false;
        if (idx == 63)
            torre_h1 = false;
    }
    if (pezzo == (pezzi::torre | pezzi::nero))
    {
        if (idx == 0)
            torre_a8 = false;
        if (idx == 7)
            torre_h8 = false;
    }

    if (idx2 == 56)
        torre_a1 = false; // torre bianca a1 catturata
    if (idx2 == 63)
        torre_h1 = false; // torre bianca h1 catturata
    if (idx2 == 0)
        torre_a8 = false; // torre nera  a8 catturata
    if (idx2 == 7)
        torre_h8 = false; // torre nera  h8 catturata

    // promozione
    int riga2 = idx2 / 8;
    if (pezzo == (pezzi::pedone | turno) && (riga2 == 0 || riga2 == 7))
    {
        promozione = true;
        idx_promozione = idx2;
        stato.era_promozione = true;
    }

    // cambio turno
    if (!promozione)
    {
        turno = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
    }
    mossa = false;
    // esegui_mossa_bot non aggiorna mai mossaC/R né bitboard né history:
    // è usata solo per validazione interna (pseudolegali) e ricerca del motore.

    if (motore_in_ricerca)
        stack_motore.push_back(stato);
    else
        storia_mosse.push_back(stato);

    return true;
}

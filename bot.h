#pragma once
#include <cstdlib>
#include <utility>
#include "scacchiera.h"

class valore_pezzi {
public:
    static int pedone;
    static int cavallo;
    static int alfiere;
    static int torre;
    static int regina;
    static int re;
};

struct Mossa {
    int da;
    int a;
    int punteggio;
    int promozione;  // 0 = nessuna, altrimenti pezzi::regina/torre/alfiere/cavallo
};

enum BandieraTT { TT_ESATTO, TT_ALFA, TT_BETA };

struct EntrataTT {
    uint64_t hash;
    int      profondita;
    int      valore;
    BandieraTT bandiera;
    int      migliore_da;
    int      migliore_a;
};

static const int DIMENSIONE_TT = 1 << 20;
extern EntrataTT tabella_trasposizioni[DIMENSIONE_TT];

static const int PROFONDITA_MASSIMA = 32;
extern int mosse_killer[PROFONDITA_MASSIMA][2];

extern int tabella_storia[2][64][64];

extern int DEPTH;

int  minimax(scacchiera &scacch, int profondita, int alfa, int beta, int colore_bot, int livello);
int  ricerca_quiete(scacchiera &scacch, int alfa, int beta, int colore_bot, int profondita_qs = 0);
int  valore_pezzo(int pezzo);
int  valutazione(scacchiera &scacch, int colore_bot);
std::pair<int,int> valuta_mossa();
int  genera_mosse(Mossa mosse[], int turno_loc, scacchiera scacch);
void ordina_mosse(Mossa mosse[], int contatore, int livello, int tt_da, int tt_a, scacchiera &scacch);
int  vittima_attaccante(int attaccante, int vittima);
void aggiorna_killer(int livello, int da, int a);
int  scegli_promozione(scacchiera &scacch, int casella, int colore_bot);
extern int pezzo_promozione_scelto;
void autoMove();

// ── Temperamento: rumore random per differenziare i bot ─────────────────────
extern int temperamento_bianco;
extern int temperamento_nero;

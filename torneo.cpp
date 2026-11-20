#include "torneo.h"
#include "bot.h"
#include "variabili.h"
#include "scacchiera.h"
#include "pezzi.h"
#include "raylib.h"
#include "review_bridge.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <iostream>

BotTorneo   bot_lista[NUM_BOT];
Partita     bracket[NUM_ROUND][8];
int         partite_per_round[NUM_ROUND] = {4, 2, 1};
StatoTorneo stato_torneo     = ST_MOSTRA_BRACKET;
int         round_corrente   = 0;
int         match_corrente   = 0;
double      tempo_inizio_torneo = 0.0;
float       bracket_timer    = BRACKET_PAUSE;
float       game_delay_timer = 0.0f;
int         campione_idx     = -1;

static const char* NOMI[NUM_BOT] = {
    "Bardellotto","Boso","Brucato","Fiorindo",
    "Xhemali","Tonu","Moretto","Solinas"
};

// ─── Disegna testo con effetto grassetto (doppio draw +1px) ──────────────────
static void DrawTextBold(const char* txt, int x, int y, int size, Color col) {
    DrawText(txt, x+1, y,   size, col);
    DrawText(txt, x,   y+1, size, col);
    DrawText(txt, x,   y,   size, col);
}

// ─────────────────────────────────────────────────────────────────────────────
void inizializza_torneo() {
    for (int i = 0; i < NUM_BOT; i++) {
        bot_lista[i].nome = NOMI[i];
        bot_lista[i].seed = 1000 + i * 137;
    }
    srand(42);
    for (int i = NUM_BOT-1; i > 0; i--) {
        int j = rand() % (i+1);
        std::swap(bot_lista[i], bot_lista[j]);
    }
    for (int r = 0; r < NUM_ROUND; r++)
        for (int m = 0; m < 8; m++)
            bracket[r][m] = {-1,-1,0,0,0,-1,false};
    for (int m = 0; m < 4; m++) {
        bracket[0][m].idx_bianco = m*2;
        bracket[0][m].idx_nero   = m*2+1;
    }
    round_corrente   = 0;
    match_corrente   = 0;
    bracket_timer    = BRACKET_PAUSE;
    game_delay_timer = 0.0f;
    stato_torneo     = ST_MOSTRA_BRACKET;
    tempo_inizio_torneo = GetTime();
}

// ─────────────────────────────────────────────────────────────────────────────
void prepara_game() {
    review_reset();
    Partita &p = bracket[round_corrente][match_corrente];

    memset(tabella_trasposizioni, 0, sizeof(tabella_trasposizioni));
    memset(mosse_killer,          0, sizeof(mosse_killer));
    memset(tabella_storia,        0, sizeof(tabella_storia));

    srand(bot_lista[p.idx_bianco].seed ^
          bot_lista[p.idx_nero  ].seed ^
          (p.games_played * 31337)     ^
          (unsigned)time(nullptr));
    temperamento_bianco = TEMPERAMENTO_CP;
    temperamento_nero   = TEMPERAMENTO_CP;

    stringa_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR wKQkq";
    fen();
    storia_mosse.clear();
    history.clear();
    matto        = false;
    draw         = false;
    fifty_move   = 0;
    mossa_fatta  = false;
    selezione    = false;
    promozione   = false;
    aggiorna_mosse = false;
    mossaC = mossaR = mossaC_a = mossaR_a = -1;
    bot_bianco   = true;
    menu         = false;
}

// ─────────────────────────────────────────────────────────────────────────────
void on_game_finito(int colore_perdente) {
    Partita &p = bracket[round_corrente][match_corrente];
    p.games_played++;

    if (colore_perdente == pezzi::bianco)      p.wins_nero++;
    else if (colore_perdente != -1)             p.wins_bianco++;

    int limite = (round_corrente == NUM_ROUND-1) ? PARTITE_FINALE : PARTITE_MATCH;
    int magg   = limite / 2 + 1;

    bool finito  = false;
    int  win_idx = -1;

    if (p.wins_bianco >= magg) {
        win_idx = p.idx_bianco; finito = true;
    } else if (p.wins_nero >= magg) {
        win_idx = p.idx_nero;   finito = true;
    } else if (p.games_played >= limite) {
        if      (p.wins_bianco > p.wins_nero) win_idx = p.idx_bianco;
        else if (p.wins_nero > p.wins_bianco) win_idx = p.idx_nero;
        else win_idx = (rand()%2==0) ? p.idx_bianco : p.idx_nero;
        finito = true;
    }

    bool mostra_bracket = (p.games_played % BRACKET_OGNI == 0) || finito;

    if (finito) {
        p.vincitore = win_idx;
        p.giocata   = true;
        std::cout << "Match vinto da: " << bot_lista[win_idx].nome
                  << " (" << p.wins_bianco << "-" << p.wins_nero << ")\n";

        if (round_corrente < NUM_ROUND-1) {
            int nm = match_corrente / 2;
            if (match_corrente % 2 == 0)
                bracket[round_corrente+1][nm].idx_bianco = win_idx;
            else
                bracket[round_corrente+1][nm].idx_nero   = win_idx;
        } else {
            campione_idx = win_idx;
        }

        match_corrente++;
        if (match_corrente >= partite_per_round[round_corrente]) {
            round_corrente++;
            match_corrente = 0;
            if (round_corrente >= NUM_ROUND) {
                stato_torneo  = ST_FINITO;
                bracket_timer = BRACKET_PAUSE;
                return;
            }
        }
    }

    if (mostra_bracket) {
        stato_torneo  = ST_MOSTRA_BRACKET;
        bracket_timer = BRACKET_PAUSE;
    } else {
        // ── 5 secondi di pausa prima del prossimo game dello stesso match ──
        stato_torneo     = ST_DELAY_GAME;
        game_delay_timer = GAME_DELAY_SEC;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  DISEGNO BRACKET
// ═════════════════════════════════════════════════════════════════════════════
static const Color C_BG      = {48,  46,  43,  255};
static const Color C_BORDER  = {80,  78,  74,  255};
static const Color C_TEXT    = {220, 220, 210,  255};
static const Color C_DIM     = {130, 130, 120,  255};
static const Color C_WIN     = {129, 183,  77,  255};
static const Color C_CURR    = {205, 209,  90,  255};

// ── Colori slot bianco / nero ─────────────────────────────────────────────
static const Color C_SLOT_W_BG  = {235, 230, 210,  255};  // panna chiaro
static const Color C_SLOT_W_TXT = { 30,  28,  25,  255};  // quasi nero
static const Color C_SLOT_W_BRD = {180, 175, 155,  255};

static const Color C_SLOT_B_BG  = { 38,  36,  33,  255};  // quasi nero
static const Color C_SLOT_B_TXT = {200, 198, 188,  255};  // grigio chiaro
static const Color C_SLOT_B_BRD = { 70,  68,  64,  255};

static const Color C_WIN_W_BG   = { 60,  80,  45,  255};  // verde scuro su bianco
static const Color C_WIN_B_BG   = { 30,  55,  25,  255};  // verde scuro su nero
static const Color C_LOSE_BG    = { 70,  40,  40,  255};

// ─────────────────────────────────────────────────────────────────────────────
// is_white = true → slot del giocatore bianco, false → nero
static void draw_slot(int x, int y, int w, int h, int bot_idx,
                      bool is_winner, bool is_loser, bool is_current,
                      bool is_white) {
    Color bg, border, tc;

    if (bot_idx == -1) {
        bg = {55,53,50,255}; border = {70,68,64,255}; tc = C_DIM;
    } else if (is_winner) {
        bg     = is_white ? C_WIN_W_BG : C_WIN_B_BG;
        border = C_WIN;
        tc     = C_WIN;
    } else if (is_loser) {
        bg = C_LOSE_BG; border = {90,50,50,255}; tc = C_DIM;
    } else if (is_current) {
        bg     = is_white ? Color{200,195,150,255} : Color{60,58,40,255};
        border = C_CURR;
        tc     = is_white ? C_SLOT_W_TXT : C_TEXT;
    } else {
        bg     = is_white ? C_SLOT_W_BG   : C_SLOT_B_BG;
        border = is_white ? C_SLOT_W_BRD  : C_SLOT_B_BRD;
        tc     = is_white ? C_SLOT_W_TXT  : C_SLOT_B_TXT;
    }

    DrawRectangleRounded({(float)x,(float)y,(float)w,(float)h}, 0.15f, 4, bg);
    DrawRectangleRoundedLines({(float)x,(float)y,(float)w,(float)h}, 0.15f, 4, border);

    // Pallino colore pezzo (piccolo cerchio a sinistra)
    int cx_dot = x + 10, cy_dot = y + h/2;
    if (bot_idx != -1) {
        if (is_white) {
            DrawCircle(cx_dot, cy_dot, 5, WHITE);
            DrawCircleLines(cx_dot, cy_dot, 5, {120,120,100,255});
        } else {
            DrawCircle(cx_dot, cy_dot, 5, {20,20,20,255});
            DrawCircleLines(cx_dot, cy_dot, 5, {120,120,100,255});
        }
    }

    const char* nome = (bot_idx == -1) ? "TBD" : bot_lista[bot_idx].nome.c_str();
    int fs = (w > 160) ? 13 : 10;
    int tx = x + 20;                          // lascia spazio al pallino
    int ty = y + (h - fs) / 2;
    DrawTextBold(nome, tx, ty, fs, tc);
}

// ─────────────────────────────────────────────────────────────────────────────
void disegna_bracket(int sw, int sh) {
    ClearBackground(C_BG);

    // ── Titolo ───────────────────────────────────────────────────────────────
    const char* tit = "TORNEO  —  ELIMINAZIONE DIRETTA";
    DrawTextBold(tit, (sw - MeasureText(tit,22))/2, 8, 22, C_TEXT);

    // ── Timer torneo ─────────────────────────────────────────────────────────
    double rem = DURATA_TORNEO_SEC - (GetTime() - tempo_inizio_torneo);
    if (rem < 0) rem = 0;
    int ore=(int)(rem/3600), min_=(int)(rem/60)%60, sec=(int)rem%60;
    const char* ts = TextFormat("Tempo rimanente:  %02d:%02d:%02d", ore, min_, sec);
    DrawText(ts, (sw - MeasureText(ts,13))/2, 36, 13, C_DIM);

    // ── Sottotitolo stato ─────────────────────────────────────────────────────
    if (stato_torneo == ST_MOSTRA_BRACKET) {
        const char* msg = TextFormat("Prossimo game tra  %.0f s",
                                     (double)(bracket_timer > 0 ? bracket_timer : 0));
        DrawTextBold(msg, (sw - MeasureText(msg,15))/2, 56, 15, C_CURR);
    } else if (stato_torneo == ST_DELAY_GAME) {
        const char* msg = TextFormat("Prossimo game tra  %.0f s",
                                     (double)(game_delay_timer > 0 ? game_delay_timer : 0));
        DrawTextBold(msg, (sw - MeasureText(msg,15))/2, 56, 15, C_CURR);
    } else if (stato_torneo == ST_FINITO) {
        DrawTextBold("TORNEO TERMINATO",
                     (sw - MeasureText("TORNEO TERMINATO",15))/2, 56, 15, C_WIN);
    }

    // ── Layout bracket ────────────────────────────────────────────────────────
    int mt   = 82;
    int bh   = sh - mt - 20;
    int col_w = 155, box_h = 34;
    int gap  = (sw - 4*col_w) / 5; if (gap < 6) gap = 6;
    int col_x[4];
    for (int c = 0; c < 4; c++) col_x[c] = gap + c*(col_w+gap);

    static const char* RN[] = {"QUARTI","SEMIFINALI","FINALE"};

    for (int r = 0; r < NUM_ROUND; r++) {
        int np   = partite_per_round[r];
        int span = bh / np;
        int cx   = col_x[r];

        // Intestazione round
        DrawTextBold(RN[r], cx + (col_w - MeasureText(RN[r],10))/2, mt-18, 10, C_DIM);

        for (int m = 0; m < np; m++) {
            Partita &p = bracket[r][m];
            int y1 = mt + m*span + span/4 - box_h/2;
            int y2 = mt + m*span + 3*span/4 - box_h/2;
            int ym = mt + m*span + span/2;

            bool curr = (r == round_corrente && m == match_corrente
                         && stato_torneo == ST_IN_PARTITA);

            bool w1 = p.giocata && p.vincitore == p.idx_bianco;
            bool l1 = p.giocata && p.vincitore != p.idx_bianco && p.idx_bianco != -1;
            bool w2 = p.giocata && p.vincitore == p.idx_nero;
            bool l2 = p.giocata && p.vincitore != p.idx_nero   && p.idx_nero   != -1;

            draw_slot(cx, y1, col_w, box_h, p.idx_bianco, w1, l1, curr, true);
            draw_slot(cx, y2, col_w, box_h, p.idx_nero,   w2, l2, curr, false);

            // Score sotto i nomi durante il match
            if (!p.giocata && (p.idx_bianco != -1 || p.idx_nero != -1)) {
                int lim = (r == NUM_ROUND-1) ? PARTITE_FINALE : PARTITE_MATCH;
                const char* sc = TextFormat("%d - %d  (%d/%d)",
                    p.wins_bianco, p.wins_nero, p.games_played, lim);
                DrawText(sc, cx, y1 - 13, 9, C_DIM);
            }

            // Linee connessione
            int lx  = cx + col_w;
            int ly1 = y1 + box_h/2, ly2 = y2 + box_h/2;
            Color lc = p.giocata ? C_WIN : C_BORDER;
            DrawLine(lx,       ly1, lx+gap/2, ly1, lc);
            DrawLine(lx,       ly2, lx+gap/2, ly2, lc);
            DrawLine(lx+gap/2, ly1, lx+gap/2, ly2, lc);
            if (r < NUM_ROUND-1) {
                int ns = bh / partite_per_round[r+1];
                int ny = mt + (m/2)*ns + ns/2;
                DrawLine(lx+gap/2, ym, col_x[r+1], ny, lc);
            }
        }
    }

    // ── Colonna campione ──────────────────────────────────────────────────────
    {
        int cx = col_x[3];
        DrawTextBold("CAMPIONE",
                     cx + (col_w - MeasureText("CAMPIONE",11))/2, mt-18, 11, C_CURR);
        int cy = mt + bh/2 - (box_h+8)/2;
        draw_slot(cx, cy, col_w, box_h+8, campione_idx,
                  campione_idx != -1, false, false, true);
        if (campione_idx != -1)
            DrawTextBold("👑 CAMPIONE!", cx, cy-22, 13, C_CURR);

        Partita &pf = bracket[NUM_ROUND-1][0];
        int ns = bh, ny = mt + ns/2;
        DrawLine(col_x[NUM_ROUND-1]+col_w, ny, cx, cy+box_h/2,
                 pf.giocata ? C_WIN : C_BORDER);
    }

    // ── Banner campione finale ────────────────────────────────────────────────
    if (stato_torneo == ST_FINITO && campione_idx != -1) {
        const char* msg = TextFormat("🏆  CAMPIONE:  %s", bot_lista[campione_idx].nome.c_str());
        int mw = MeasureText(msg, 36);
        DrawRectangle((sw-mw-60)/2, sh/2-48, mw+60, 96, Fade(BLACK, 0.90f));
        DrawRectangleLines((sw-mw-60)/2, sh/2-48, mw+60, 96, C_WIN);
        DrawTextBold(msg, (sw-mw)/2, sh/2-18, 36, C_WIN);
    }
}

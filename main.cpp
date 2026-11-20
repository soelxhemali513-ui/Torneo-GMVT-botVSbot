#include <iostream>
#include "raylib.h"
#include "pezzi.h"
#include "scacchiera.h"
#include "variabili.h"
#include "grafica.h"
#include "statistiche.h"
#include "bot.h"
#include "torneo.h"
#include "review_bridge.h"
#include <string>

// ── Costanti layout ──────────────────────────────────────────────────────────
static const int PADDING = 24;
static const int GAP = 12;

static const Color BG_COLOR = {30, 28, 25, 255};
static const Color PANEL_BORDER = {70, 68, 64, 255};
static const Color TEXT_PRIMARY = {220, 220, 210, 255};
static const Color TEXT_DIM = {130, 130, 120, 255};
static const Color ACCENT_GREEN = {129, 183, 77, 255};
static const Color ACCENT_YELLOW = {205, 209, 90, 255};

static int s_px = 0;
static int s_pw = 300;

static Texture2D tex_qr;
static Texture2D tex_gmvt;

static void updateLayout()
{
    int sw = GetScreenWidth(), sh = GetScreenHeight();

    // Pannello laterale: larghezza fissa ~280px, mai meno di 160
    static const int PANEL_W = 280;
    int pw = (sw > 600) ? PANEL_W : 160;

    // Scacchiera: occupa tutto lo spazio a sinistra del pannello
    int avail_w = sw - 3 * PADDING - GAP - pw; // spazio orizzontale per la board
    int avail_h = sh - 2 * PADDING;            // spazio verticale per la board
    if (avail_w < 8)
        avail_w = 8; // guard contro valori degeneri
    if (avail_h < 8)
        avail_h = 8;

    // La board è il minore dei due assi disponibili → quadrato perfetto
    int board_px = (avail_w < avail_h) ? avail_w : avail_h;
    if (board_px < 200)
        board_px = 200;
    board_px = (board_px / 8) * 8; // multiplo di 8
    sq_size = board_px / 8;

    // Centra la scacchiera dentro lo spazio disponibile (non su sh intero)
    board_offset_x = PADDING + (avail_w - board_px) / 2;
    if (board_offset_x < PADDING)
        board_offset_x = PADDING;

    board_offset_y = PADDING + (avail_h - board_px) / 2;
    if (board_offset_y < PADDING)
        board_offset_y = PADDING;
    // Clamp esplicito: il bordo inferiore non deve mai uscire dallo schermo.
    // Necessario quando GetScreenHeight() restituisce un valore leggermente
    // diverso dalla reale area disegnabile (DPI scaling, monitor non standard).
    if (board_offset_y + board_px > sh - PADDING)
        board_offset_y = sh - PADDING - board_px;
    if (board_offset_y < 0)
        board_offset_y = 0;

    // Pannello a destra
    s_px = sw - PADDING - pw;
    s_pw = pw;
    panel_x_dyn = s_px;
    panel_w_dyn = s_pw;
}

static void draw_card(int y, int h, Color bg = {45, 43, 40, 255})
{
    DrawRectangleRounded({(float)s_px, (float)y, (float)s_pw, (float)h}, 0.10f, 6, bg);
    DrawRectangleRoundedLines({(float)s_px, (float)y, (float)s_pw, (float)h}, 0.10f, 6, PANEL_BORDER);
}

static void controlla_fine_game()
{
    if (matto)
        on_game_finito(turno);
    else if (draw)
        on_game_finito(-1);
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 800, "Torneo Scacchi");
    // Adatta la finestra alla risoluzione nativa del monitor PRIMA di andare
    // fullscreen. Senza questo, su monitor con altezza < 800px (es. 1366×768
    // o 1280×720) ToggleFullscreen eredita le dimensioni 1280×800 e
    // GetScreenHeight() continua a restituire 800, causando il taglio della
    // board in basso.
    {
        int mon = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(mon), GetMonitorHeight(mon));
    }
    ToggleFullscreen();
    SetTargetFPS(60);

    // ── Audio ────────────────────────────────────────────────────────────────
    InitAudioDevice();
    suono_move = LoadSound("audio/move-self.mp3");
    suono_promozione = LoadSound("audio/promotion.mp3");
    suono_capture = LoadSound("audio/capture.mp3");
    suono_scacco = LoadSound("audio/move-check.mp3");
    suono_arrocco = LoadSound("audio/castle.mp3");

    inizializza_grafica();
    tex_qr = LoadTexture("pezzi/qrcode_gmvt.png");
    tex_gmvt = LoadTexture("pezzi/biglietto_GMVT.png");
    review_init();
    inizializza_torneo();

    while (!WindowShouldClose())
    {
        updateLayout();
        float dt = GetFrameTime();
        int sq = sq_size;

        if (IsKeyPressed(KEY_F11) ||
            (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER)))
            ToggleFullscreen();

        // ══ Schermate bracket / delay ════════════════════════════════════════
        if (stato_torneo == ST_MOSTRA_BRACKET)
        {
            bracket_timer -= dt;
            if (bracket_timer <= 0.0f)
            {
                prepara_game();
                stato_torneo = ST_IN_PARTITA;
            }
            BeginDrawing();
            disegna_bracket(GetScreenWidth(), GetScreenHeight());
            EndDrawing();
            continue;
        }
        if (stato_torneo == ST_DELAY_GAME)
        {
            game_delay_timer -= dt;
            if (game_delay_timer <= 0.0f)
            {
                prepara_game();
                stato_torneo = ST_IN_PARTITA;
            }
            BeginDrawing();
            disegna_bracket(GetScreenWidth(), GetScreenHeight());
            EndDrawing();
            continue;
        }
        if (stato_torneo == ST_FINITO)
        {
            BeginDrawing();
            disegna_bracket(GetScreenWidth(), GetScreenHeight());
            EndDrawing();
            continue;
        }

        // ══ ST_IN_PARTITA ════════════════════════════════════════════════════
        avversario = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
        partita();
        avversario = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;

        if (!promozione && !mossa_fatta)
        {
            autoMove();
            risultato_partita();
            if (!matto && !draw)
                mossa_fatta = false;
        }
        controlla_fine_game();

        // ── Disegno ──────────────────────────────────────────────────────────
        BeginDrawing();
        ClearBackground(BG_COLOR);

        int ox = board_offset_x, oy = board_offset_y;
        int board_px = sq * 8;

        disegna_scacchiera(righe, colonne, larghezza_case, altezza_case, case_bianche, case_scure);

        if (mossaC >= 0)
            DrawRectangle(ox + mossaC * sq, oy + mossaR * sq, sq, sq, {205, 178, 90, 160});
        if (mossaC_a >= 0)
            DrawRectangle(ox + mossaC_a * sq, oy + mossaR_a * sq, sq, sq, {205, 178, 90, 160});

        disegna_pezzi(b, righe, colonne, larghezza_case, altezza_case,
                      re_bianco, re_nero, pedone_bianco, pedone_nero,
                      cavallo_bianco, cavallo_nero, alfiere_bianco, alfiere_nero,
                      torre_bianca, torre_nera, regina_bianca, regina_nera);

        if (mossaC_a >= 0 && mossaR_a >= 0)
            review_draw_badge(mossaC_a, mossaR_a, ox, oy, sq);

        // ── QR code + logo GMVT (a sinistra della scacchiera, in alto) ─────────
        if (tex_qr.id > 0 || tex_gmvt.id > 0)
        {
            // Spazio disponibile a sinistra della board: 0..ox
            int avail_left = ox - 8;
            if (avail_left < 20)
                avail_left = 20;
            int widget_w = avail_left;
            if (widget_w > sq * 2)
                widget_w = sq * 2;

            int qr_sz = widget_w;
            int logo_w = widget_w;
            int logo_h = (tex_gmvt.id > 0 && tex_gmvt.width > 0)
                             ? (int)((float)logo_w * tex_gmvt.height / tex_gmvt.width)
                             : widget_w / 3;

            // Centrato orizzontalmente nello spazio a sinistra
            int base_x = (ox - widget_w) / 2;
            if (base_x < 4)
                base_x = 4;
            int base_y = oy;
            int gap = 6;

            // Logo GMVT in alto
            if (tex_gmvt.id > 0)
            {
                Rectangle src = {0, 0, (float)tex_gmvt.width, (float)tex_gmvt.height};
                Rectangle dst = {(float)base_x, (float)base_y, (float)logo_w, (float)logo_h};
                DrawTexturePro(tex_gmvt, src, dst, {0, 0}, 0.0f, WHITE);
            }
            // QR code sotto il logo
            if (tex_qr.id > 0)
            {
                int qr_y = base_y + logo_h + gap;
                Rectangle src = {0, 0, (float)tex_qr.width, (float)tex_qr.height};
                Rectangle dst = {(float)base_x, (float)qr_y, (float)qr_sz, (float)qr_sz};
                DrawTexturePro(tex_qr, src, dst, {0, 0}, 0.0f, WHITE);
                // Scritta sotto il QR
                const char *label = "SCOPRI IL BOT!";
                int fs = (widget_w > 80) ? 14 : 10;
                int lw = MeasureText(label, fs);
                int lx = base_x + (qr_sz - lw) / 2;
                DrawText(label, lx, qr_y + qr_sz + 4, fs, ACCENT_GREEN);
            }
        }

        if (matto)
        {
            DrawRectangle(ox, oy + board_px / 2 - 60, board_px, 120, Fade(BLACK, 0.75f));
            const char *msg = (turno == pezzi::bianco) ? "VINCE IL NERO" : "VINCE IL BIANCO";
            int fs = sq >= 80 ? 52 : 36;
            DrawText(msg, ox + (board_px - MeasureText(msg, fs)) / 2, oy + board_px / 2 - fs / 2, fs, ACCENT_GREEN);
        }
        else if (draw)
        {
            DrawRectangle(ox, oy + board_px / 2 - 60, board_px, 120, Fade(BLACK, 0.75f));
            int fs = sq >= 80 ? 52 : 36;
            DrawText("PAREGGIO", ox + (board_px - MeasureText("PAREGGIO", fs)) / 2,
                     oy + board_px / 2 - fs / 2, fs, ACCENT_YELLOW);
        }

        // ── Pannello ─────────────────────────────────────────────────────────
        if (s_pw >= 120)
        {
            Partita &pm = bracket[round_corrente][match_corrente];
            static const char *RN[] = {"Sedicesimi", "Quarti", "Semifinali", "Finale"};
            int lim = (round_corrente == NUM_ROUND - 1) ? PARTITE_FINALE : PARTITE_MATCH;

            // Altezze di riferimento a 800px di schermo (design base)
            // card0=50 gap=12 | card1=58 gap=12 | card2=70 gap=12 |
            // card3=36 gap=12 | card4=88 gap=12 | card5=44 gap=12 |
            // card6=30 gap=12 | card7=26 gap=12 | review=resto
            static const int H_BASE[] = {50, 58, 70, 36, 88, 30, 26};
            static const int N_CARDS = 7;
            static const int GAP_CARD = 8;
            static const int REVIEW_MIN = 50;
            static const int PANEL_H_BASE = 800 - 2 * PADDING; // spazio verticale a 800px

            int panel_avail = GetScreenHeight() - 2 * PADDING;
            if (panel_avail < 100)
                panel_avail = 100;

            // Somma altezze fisse + gap tra card
            int fixed_total = (N_CARDS - 1) * GAP_CARD;
            for (int i = 0; i < N_CARDS; i++)
                fixed_total += H_BASE[i];

            // Spazio residuo per la review
            int review_base = PANEL_H_BASE - fixed_total - GAP_CARD;
            if (review_base < REVIEW_MIN)
                review_base = REVIEW_MIN;
            int total_base = fixed_total + GAP_CARD + review_base;

            // Fattore di scala: adatta tutto allo spazio disponibile
            float sc_f = (float)panel_avail / (float)total_base;
            if (sc_f > 1.0f)
                sc_f = 1.0f; // non ingrandire oltre il design base

            auto S = [&](int v)
            { return (int)(v * sc_f); }; // scala una misura
            auto FS = [&](int v)
            { return S(v) < 6 ? 6 : S(v); }; // font min 6

            int cy = PADDING; // parte SEMPRE da PADDING, indipendente dalla board

            // Card 0 – titolo
            draw_card(cy, S(50));
            DrawText("TORNEO SCACCHI", s_px + S(10), cy + S(8), FS(14), TEXT_PRIMARY);
            DrawText("bot vs bot", s_px + S(10), cy + S(28), FS(10), TEXT_DIM);
            cy += S(50) + GAP_CARD;

            // Card 1 – round/match/game
            draw_card(cy, S(58));
            DrawText(TextFormat("Round: %s", RN[round_corrente]),
                     s_px + S(10), cy + S(6), FS(12), TEXT_PRIMARY);
            DrawText(TextFormat("Match %d/%d", match_corrente + 1,
                                partite_per_round[round_corrente]),
                     s_px + S(10), cy + S(24), FS(10), TEXT_DIM);
            DrawText(TextFormat("Game %d/%d", pm.games_played + 1, lim),
                     s_px + S(10), cy + S(40), FS(10), ACCENT_YELLOW);
            cy += S(58) + GAP_CARD;

            // Card 2 – bot nomi/punteggi
            draw_card(cy, S(70));
            const char *nb = pm.idx_bianco >= 0 ? bot_lista[pm.idx_bianco].nome.c_str() : "?";
            const char *nn = pm.idx_nero >= 0 ? bot_lista[pm.idx_nero].nome.c_str() : "?";
            DrawCircle(s_px + S(16), cy + S(17), S(7), WHITE);
            DrawText(nb, s_px + S(28), cy + S(10), FS(12), TEXT_PRIMARY);
            DrawText(TextFormat("%d", pm.wins_bianco), s_px + s_pw - S(24), cy + S(10), FS(13), ACCENT_GREEN);
            DrawCircle(s_px + S(16), cy + S(47), S(7), {28, 28, 28, 255});
            DrawCircleLines(s_px + S(16), cy + S(47), S(7), TEXT_DIM);
            DrawText(nn, s_px + S(28), cy + S(40), FS(12), TEXT_PRIMARY);
            DrawText(TextFormat("%d", pm.wins_nero), s_px + s_pw - S(24), cy + S(40), FS(13), ACCENT_GREEN);
            cy += S(70) + GAP_CARD;

            // Card 3 – turno
            draw_card(cy, S(36));
            bool isW = (turno == pezzi::bianco);
            DrawCircle(s_px + S(16), cy + S(18), S(7), isW ? WHITE : Color{28, 28, 28, 255});
            DrawCircleLines(s_px + S(16), cy + S(18), S(7), TEXT_DIM);
            DrawText(isW ? "TURNO: BIANCO" : "TURNO: NERO", s_px + S(30), cy + S(11), FS(12), TEXT_PRIMARY);
            cy += S(36) + GAP_CARD;

            // Card 4 – motore
            draw_card(cy, S(88));
            DrawText("MOTORE", s_px + S(10), cy + S(6), FS(10), TEXT_DIM);
            DrawText(TextFormat("Nodi: %lld", nodes_searched), s_px + S(10), cy + S(20), FS(10), TEXT_PRIMARY);
            DrawText(TextFormat("Nodi/s: %d", nodes_per_second), s_px + S(10), cy + S(36), FS(10), ACCENT_GREEN);
            DrawText(TextFormat("Depth: %d", profondita_raggiunta), s_px + S(10), cy + S(52), FS(10), {100, 180, 230, 255});
            DrawText(TextFormat("Ms: %d", tempo_ultima_mossa_ms), s_px + S(10), cy + S(68), FS(10), ACCENT_YELLOW);
            cy += S(88) + GAP_CARD;

            // Card 6 – bracket countdown
            draw_card(cy, S(30));
            DrawText(TextFormat("Bracket tra %d game",
                                BRACKET_OGNI - (pm.games_played % BRACKET_OGNI)),
                     s_px + S(10), cy + S(9), FS(10), TEXT_DIM);
            cy += S(30) + GAP_CARD;

            // Card 7 – shortcut
            draw_card(cy, S(26));
            DrawText("F11", s_px + S(10), cy + S(7), FS(11), ACCENT_GREEN);
            DrawText("fullscreen", s_px + S(40), cy + S(7), FS(11), TEXT_DIM);
            cy += S(26) + GAP_CARD;

            // Review – prende tutto lo spazio rimasto fino al fondo
            int review_h = GetScreenHeight() - cy - PADDING;
            if (review_h > REVIEW_MIN)
                review_draw(s_px, cy, s_pw, review_h);
        }

        EndDrawing();
    }

    // ── Cleanup audio ────────────────────────────────────────────────────────
    UnloadSound(suono_move);
    UnloadSound(suono_promozione);
    UnloadSound(suono_capture);
    UnloadSound(suono_scacco);
    UnloadSound(suono_arrocco);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}

//  g++ main.cpp scacchiera.cpp variabili.cpp grafica.cpp bot.cpp autoMoveBot.cpp aperture.cpp statistiche.cpp torneo.cpp review_bridge.cpp review/StockfishBridge.cpp libraylib.a -o torneo.exe -I. -lopengl32 -lgdi32 -lwinmm -std=c++17 -O2 -static -static-libgcc -static-libstdc++ -O2
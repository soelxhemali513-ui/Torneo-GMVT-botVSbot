#pragma once
#include "MoveReviewer.h"
#include "raylib.h"
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include <sstream>

// Converte RLColor (nostra struct) in Color di raylib
inline Color toRayColor(RLColor c) { return {c.r, c.g, c.b, c.a}; }

// ============================================================
// ReviewPanel — UI Raylib che riproduce il look di WintrChess
//
// Usa le PNG originali del sito se le copi in img/classifications/
// Altrimenti fallback su colori + simboli testuali.
//
// Layout:
//   [EvalBar 20px | Intestazione | Lista mosse scrollabile]
// ============================================================

// Converte eval centipawn in stringa tipo "+1.3" / "-0.5" / "M4"
inline std::string evalToString(const Evaluation& e) {
    if (e.type == Evaluation::Type::MATE) {
        if (e.value == 0) return "0.0";
        return std::string(e.value > 0 ? "M" : "-M") + std::to_string(std::abs(e.value));
    }
    float cp = (float)e.value / 100.0f;
    char buf[16];
    snprintf(buf, sizeof(buf), "%+.1f", cp);
    return buf;
}

class ReviewPanel {
public:
    // Carica le icone PNG del sito (opzionale).
    // Chiama dopo InitWindow().
    // basePath: cartella contenente le PNG (es. "img/classifications/")
    void loadIcons(const std::string& basePath = "img/classifications/") {
        const Classification all[] = {
            Classification::BRILLIANT, Classification::CRITICAL,
            Classification::BEST,      Classification::EXCELLENT,
            Classification::OKAY,      Classification::INACCURACY,
            Classification::MISTAKE,   Classification::BLUNDER,
            Classification::FORCED,    Classification::THEORY,
            Classification::RISKY
        };
        for (auto c : all) {
            std::string path = basePath + classificationIconPath(c).substr(
                classificationIconPath(c).find_last_of('/') + 1
            );
            if (FileExists(path.c_str())) {
                Texture2D tex = LoadTexture(path.c_str());
                icons[c] = tex;
            }
        }
        iconsLoaded = true;
    }

    void unloadIcons() {
        for (auto& [c, tex] : icons)
            UnloadTexture(tex);
        icons.clear();
    }

    // Disegna l'icona PNG della classificazione nell'angolo in alto a destra
    // della casa. Identica alle icone di chess.com sul pezzo.
    void drawBadge(Classification c, int col, int row, int ox, int oy, int sq) {
        int sz = std::max(16, sq / 4);
        int bx = ox + col * sq + sq - sz - 2;
        int by = oy + row * sq + 2;

        if (iconsLoaded && icons.count(c)) {
            Texture2D& tex = icons.at(c);
            Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
            Rectangle dst = { (float)bx, (float)by, (float)sz, (float)sz };
            DrawTexturePro(tex, src, dst, {0,0}, 0.0f, WHITE);
        } else {
            RLColor rc = classificationColor(c);
            Color   bc = {rc.r, rc.g, rc.b, rc.a};
            int r = sz / 2;
            DrawCircle(bx + r, by + r, (float)r + 1, {0,0,0,160});
            DrawCircle(bx + r, by + r, (float)r, bc);
            const char* sym = classificationSymbol(c);
            if (sym && sym[0] != '\0') {
                int tw = MeasureText(sym, 9);
                DrawText(sym, bx + r - tw/2, by + r - 5, 9, WHITE);
            }
        }
    }

    // -------------------------------------------------------
    // draw() — chiama nel loop di rendering
    //
    // bounds: rettangolo in cui disegnare il panel
    // reviewer: il MoveReviewer con le mosse classificate
    // -------------------------------------------------------
    void draw(Rectangle bounds, const MoveReviewer& reviewer) {
        const auto moves = reviewer.getMoves();
        const Evaluation eval = reviewer.getCurrentEval();

        // Sfondo panel
        DrawRectangleRec(bounds, BG_DARK);

        // --- Eval Bar (sinistra, 20px) ---
        Rectangle evalBarRect = { bounds.x, bounds.y, EVAL_BAR_W, bounds.height };
        drawEvalBar(evalBarRect, eval);

        float contentX = bounds.x + EVAL_BAR_W + PADDING;
        float contentW = bounds.width - EVAL_BAR_W - PADDING * 2;

        // --- Header: valore eval numerico ---
        std::string evalStr = evalToString(eval);
        DrawText(evalStr.c_str(), (int)contentX, (int)(bounds.y + PADDING), 16, WHITE);

        // Linea separatore
        float sepY = bounds.y + HEADER_H;
        DrawLine((int)contentX, (int)sepY,
                 (int)(contentX + contentW), (int)sepY, BG_SEPARATOR);

        // --- Lista mosse (2 colonne: bianco | nero) ---
        float listY = sepY + PADDING;
        float listH = bounds.height - HEADER_H - PADDING;
        drawMoveList(contentX, listY, contentW, listH, moves);
    }

private:
    // ---- Costanti di stile (ispirate al CSS del sito) ----
    static constexpr float EVAL_BAR_W  = 20.0f;
    static constexpr float PADDING     = 8.0f;
    static constexpr float HEADER_H    = 32.0f;
    static constexpr float ROW_H       = 28.0f;
    static constexpr float ICON_SIZE   = 18.0f;
    static constexpr int   FONT_SIZE   = 14;

    // Colori dal CSS del sito (--ui-shade-X)
    const Color BG_DARK      = { 26,  26,  28,  255 };
    const Color BG_ROW_EVEN  = { 36,  36,  38,  255 };
    const Color BG_ROW_ODD   = { 46,  46,  50,  255 };
    const Color BG_SEPARATOR = { 60,  60,  65,  255 };
    const Color TEXT_DIM     = { 140, 140, 150, 255 };
    const Color TEXT_WHITE   = WHITE;

    bool iconsLoaded = false;
    std::map<Classification, Texture2D> icons;

    // -------------------------------------------------------
    // Eval Bar verticale
    // Formula dal sito: overBarHeight = clamp(50 - eval/20, 5, 95)
    // overBarHeight = altezza della porzione NERA (dall'alto)
    // -------------------------------------------------------
    void drawEvalBar(Rectangle bar, const Evaluation& eval) {
        // Sfondo (colore del lato in vantaggio)
        DrawRectangleRec(bar, { 255, 255, 255, 255 }); // bianco sotto

        float overBarPct;
        if (eval.type == Evaluation::Type::CENTIPAWN) {
            // Identico a EvaluationBar.tsx: clamp(50 - cp/20, 5, 95)
            overBarPct = std::clamp(50.0f - (eval.value / 20.0f), 5.0f, 95.0f);
        } else {
            // Matto
            if (eval.value == 0) overBarPct = 50.0f;
            else overBarPct = eval.value > 0 ? 5.0f : 95.0f;
        }

        float blackH = bar.height * (overBarPct / 100.0f);
        DrawRectangle((int)bar.x, (int)bar.y, (int)bar.width, (int)blackH,
                      { 12, 12, 12, 255 });

        // Bordo
        DrawRectangleLinesEx(bar, 1, { 60, 60, 65, 255 });
    }

    // -------------------------------------------------------
    // Lista mosse a 2 colonne (bianco | nero) con classificazione
    // -------------------------------------------------------
    void drawMoveList(
        float x, float y, float w, float h,
        const std::vector<ReviewedMove>& moves
    ) {
        if (moves.empty()) {
            DrawText("Waiting for moves...", (int)x, (int)y, 12, TEXT_DIM);
            return;
        }

        // Numero righe visibili
        int maxVisible = (int)(h / ROW_H);
        int totalRows = (int)((moves.size() + 1) / 2); // 2 mosse per riga
        int startRow = std::max(0, totalRows - maxVisible);

        float colW = w / 2.0f;
        float numW = 30.0f; // larghezza numero mossa

        for (int row = startRow; row < totalRows; row++) {
            float rowY = y + (row - startRow) * ROW_H;

            // Sfondo alternato
            Color rowBg = (row % 2 == 0) ? BG_ROW_EVEN : BG_ROW_ODD;
            DrawRectangle((int)x, (int)rowY, (int)w, (int)ROW_H, rowBg);

            // Numero mossa (1. 2. 3. ...)
            int moveNum = row + 1;
            std::string numStr = std::to_string(moveNum) + ".";
            DrawText(numStr.c_str(), (int)x + 2, (int)(rowY + 7), 13, TEXT_DIM);

            // Mossa bianca (indice = row*2)
            int wIdx = row * 2;
            if (wIdx < (int)moves.size())
                drawMoveCell(x + numW, rowY, colW - numW, moves[wIdx]);

            // Mossa nera (indice = row*2 + 1)
            int bIdx = row * 2 + 1;
            if (bIdx < (int)moves.size())
                drawMoveCell(x + numW + colW, rowY, colW - numW, moves[bIdx]);
        }
    }

    // -------------------------------------------------------
    // Cella singola mossa: [icona] [SAN] [accuracy%]
    // -------------------------------------------------------
    void drawMoveCell(float x, float y, float w, const ReviewedMove& mv) {
        Classification c = mv.classification;
        Color clrColor = toRayColor(classificationColor(c));

        // Icona (PNG se disponibile, altrimenti testo colorato)
        float iconX = x + 2;
        float iconY = y + (ROW_H - ICON_SIZE) / 2.0f;

        if (iconsLoaded && icons.count(c)) {
            Texture2D& tex = icons.at(c);
            float scale = ICON_SIZE / (float)std::max(tex.width, tex.height);
            DrawTextureEx(tex, { iconX, iconY }, 0, scale, WHITE);
        } else {
            // Fallback testuale colorato
            const char* sym = classificationSymbol(c);
            if (sym[0] != '\0')
                DrawText(sym, (int)iconX, (int)(y + 7), 12, clrColor);
        }

        // Nome mossa (SAN)
        float textX = iconX + ICON_SIZE + 3;
        DrawText(mv.moveSAN.c_str(), (int)textX, (int)(y + 7), FONT_SIZE, TEXT_WHITE);

        // Accuracy a destra
        char accBuf[16];
        snprintf(accBuf, sizeof(accBuf), "%.0f%%", mv.accuracy);
        int accW = MeasureText(accBuf, 12);
        DrawText(accBuf, (int)(x + w - accW - 4), (int)(y + 9), 12, TEXT_DIM);
    }
};

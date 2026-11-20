// ============================================================
// ESEMPIO DI INTEGRAZIONE nel tuo loop torneo C++/Raylib
//
// Adatta i nomi delle funzioni al tuo codice esistente
// (scacchiera.toFEN(), bot.calcolaMossa(), ecc.)
// ============================================================

#include "review/MoveReviewer.h"
#include "review/ReviewPanel.h"
#include "raylib.h"

// ----------------------------------------------------------------
// SETUP (una volta sola, fuori dal loop)
// ----------------------------------------------------------------

MoveReviewer reviewer;
ReviewPanel  reviewPanel;

void setupReview() {
    // "stockfish.exe" nella stessa cartella del tuo eseguibile
    // oppure il path dove lo hai messo nel progetto
    if (!reviewer.init("stockfish.exe", 14)) {
        TraceLog(LOG_ERROR, "Impossibile avviare Stockfish!");
    }

    // Carica le icone PNG dal sito
    // Copia client/public/img/classifications/*.png in img/classifications/
    reviewPanel.loadIcons("img/classifications/");
}

// ----------------------------------------------------------------
// NEL LOOP DI GIOCO — quando il bot deve muovere
// ----------------------------------------------------------------

void onBotMovePhase(/* i tuoi parametri: scacchiera, bot, ecc. */) {

    std::string fenPrima = scacchiera.toFEN();
    int mosseLE = scacchiera.getLegalMoves().size();

    // 1. Analisi PRE-mossa (Stockfish valuta la posizione attuale)
    reviewer.onBeforeMove(fenPrima, mosseLE);

    // 2. Il bot calcola e gioca la sua mossa (il tuo codice esistente)
    Mossa m = bot.calcolaMossa(scacchiera);
    scacchiera.eseguiMossa(m);

    // 3. Analisi POST-mossa (Stockfish valuta la posizione risultante)
    std::string fenDopo = scacchiera.toFEN();
    bool isMatto = scacchiera.isCheckmate();

    reviewer.onAfterMove(
        m.toUCI(),       // es. "e2e4"
        fenDopo,
        m.toSAN(),       // es. "e4" — opzionale
        isMatto
    );

    // Opzionale: log della classificazione
    auto moves = reviewer.getMoves();
    if (!moves.empty()) {
        const auto& last = moves.back();
        TraceLog(LOG_INFO, "Mossa %s: %s (accuracy %.0f%%)",
            last.moveSAN.c_str(),
            classificationName(last.classification),
            last.accuracy
        );
    }
}

// ----------------------------------------------------------------
// NEL LOOP DI RENDERING — disegna il panel
// ----------------------------------------------------------------

void drawUI() {
    // ...il tuo codice per disegnare la scacchiera...

    // Panel di review sulla destra (adatta le coordinate al tuo layout)
    // Esempio per layout 1200x800 con scacchiera 800x800:
    Rectangle panelBounds = { 820, 0, 380, 800 };
    reviewPanel.draw(panelBounds, reviewer);
}

// ----------------------------------------------------------------
// NUOVA PARTITA
// ----------------------------------------------------------------

void onNewGame() {
    reviewer.reset();
}

// ----------------------------------------------------------------
// CLEANUP
// ----------------------------------------------------------------

void onShutdown() {
    reviewPanel.unloadIcons();
    // StockfishBridge si chiude nel destructor
}

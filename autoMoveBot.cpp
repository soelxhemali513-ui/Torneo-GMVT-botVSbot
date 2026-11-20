#include <cstdlib>
#include <iostream>
#include <ctime>
#include "variabili.h"
#include "scacchiera.h"
#include "pezzi.h"
#include "bot.h"
#include "raylib.h"
#include <utility>
#include "review_bridge.h"

static std::pair<int,int> s_mossa_pronta = {-2, -2};
static double              s_tempo_pronta = -1.0;
static const double        DELAY_MOSSA    =  1.0;

void autoMove() {
    if (mossa_fatta) return;

    if (s_mossa_pronta.first == -2) {
        review_pre_move();
        s_mossa_pronta  = valuta_mossa();
        s_tempo_pronta  = GetTime();
        selezione       = false;
        mosse_possibili = 0;
        for (int i = 0; i < 27; i++) mossa_possibile[i] = -1;
    }

    if (GetTime() - s_tempo_pronta < DELAY_MOSSA) return;

    std::pair<int,int> mossa_scelta = s_mossa_pronta;
    s_mossa_pronta = {-2, -2};
    s_tempo_pronta = -1.0;

    if (mossa_scelta.first != -1 && mossa_scelta.second != -1) {
        esegui_mossa(b, mossa_scelta.first, mossa_scelta.second, matto);
        review_post_move(mossa_scelta.first, mossa_scelta.second, matto);
        mossa_fatta = true;
        selezione   = false;

        if (promozione) {
            int pezzo_scelto = pezzo_promozione_scelto;
            if (pezzo_scelto == 0)
                pezzo_scelto = scegli_promozione(b, idx_promozione, turno);
            b.casa[idx_promozione] = pezzo_scelto | turno;
            promozione = false;
            turno = (turno == pezzi::bianco) ? pezzi::nero : pezzi::bianco;
        }
    } else if (scacco_al_re(b, turno, avversario, casa_re, enpassant, idx_enpassant,
                            re_e1, re_e8, torre_a1, torre_a8, torre_h1, torre_h8)) {
        // FIX: passa la board esplicitamente come nel bot singolo
        matto = scacco_matto(turno, avversario, b);
        if (matto) selezione = false;
    }
}

#pragma once
// fen_utils.h — genera una stringa FEN dalla scacchiera globale `b`
// Usa le stesse variabili globali di variabili.h (turno, re_e1, enpassant, ecc.)
#include "pezzi.h"
#include "variabili.h"
#include <string>
#include <cctype>

// idx 0=a8, 7=h8, 56=a1, 63=h1
inline std::string idx_to_sq(int idx) {
    char col = 'a' + (idx % 8);
    char row = '1' + (7 - idx / 8);
    return { col, row };
}

inline std::string move_to_uci(int from, int to, int promo_pezzo = 0) {
    std::string s = idx_to_sq(from) + idx_to_sq(to);
    switch (promo_pezzo & 7) {
        case pezzi::regina:  s += 'q'; break;
        case pezzi::torre:   s += 'r'; break;
        case pezzi::alfiere: s += 'b'; break;
        case pezzi::cavallo: s += 'n'; break;
    }
    return s;
}

inline std::string board_to_fen() {
    std::string fen;

    // 1. Pezzi
    for (int rank = 0; rank < 8; rank++) {
        int empty = 0;
        for (int file = 0; file < 8; file++) {
            int p = b.casa[rank * 8 + file];
            if (p == pezzi::vuoto) {
                empty++;
            } else {
                if (empty > 0) { fen += char('0' + empty); empty = 0; }
                bool bianco = (p & pezzi::bianco) != 0;
                char c = '?';
                switch (p & 7) {
                    case pezzi::pedone:  c = 'P'; break;
                    case pezzi::cavallo: c = 'N'; break;
                    case pezzi::alfiere: c = 'B'; break;
                    case pezzi::torre:   c = 'R'; break;
                    case pezzi::regina:  c = 'Q'; break;
                    case pezzi::re:      c = 'K'; break;
                }
                fen += bianco ? c : char(tolower(c));
            }
        }
        if (empty > 0) fen += char('0' + empty);
        if (rank < 7) fen += '/';
    }

    // 2. Turno
    fen += (turno == pezzi::bianco) ? " w " : " b ";

    // 3. Arrocco
    std::string castle;
    if (re_e1  && torre_h1) castle += 'K';
    if (re_e1  && torre_a1) castle += 'Q';
    if (re_e8  && torre_h8) castle += 'k';
    if (re_e8  && torre_a8) castle += 'q';
    fen += castle.empty() ? "-" : castle;

    // 4. En passant
    fen += ' ';
    if (enpassant && idx_enpassant >= 0 && idx_enpassant < 64)
        fen += idx_to_sq(idx_enpassant);
    else
        fen += '-';

    // 5. Halfmove / fullmove
    fen += ' ';
    fen += std::to_string(fifty_move);
    fen += ' ';
    fen += std::to_string((int)storia_mosse.size() / 2 + 1);

    return fen;
}

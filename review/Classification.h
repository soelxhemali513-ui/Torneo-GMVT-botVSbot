#pragma once
// Classification.h — NON include raylib.h.
// Usa RLColor (struct propria) per evitare conflitti con windows.h.
// La conversione a raylib::Color avviene solo in ReviewPanel.h.
#include <string>

enum class Classification {
    BRILLIANT, CRITICAL, BEST, EXCELLENT, OKAY,
    INACCURACY, MISTAKE, BLUNDER, FORCED, THEORY, RISKY, UNKNOWN
};

// Struct colore indipendente da raylib
struct RLColor { unsigned char r, g, b, a; };

inline RLColor classificationColor(Classification c) {
    switch (c) {
        case Classification::BRILLIANT:  return {0x1b,0xaa,0xa6,0xff};
        case Classification::CRITICAL:   return {0x5b,0x8b,0xaf,0xff};
        case Classification::BEST:       return {0x98,0xbc,0x49,0xff};
        case Classification::EXCELLENT:  return {0x98,0xbc,0x49,0xff};
        case Classification::OKAY:       return {0x97,0xaf,0x8b,0xff};
        case Classification::INACCURACY: return {0xf4,0xbf,0x44,0xff};
        case Classification::MISTAKE:    return {0xe2,0x8c,0x28,0xff};
        case Classification::BLUNDER:    return {0xc9,0x32,0x30,0xff};
        case Classification::FORCED:     return {0x97,0xaf,0x8b,0xff};
        case Classification::THEORY:     return {0xa8,0x87,0x64,0xff};
        case Classification::RISKY:      return {0x89,0x83,0xac,0xff};
        default:                         return {0x80,0x80,0x80,0xff};
    }
}

inline const char* classificationSymbol(Classification c) {
    switch (c) {
        case Classification::BRILLIANT:  return "!!";
        case Classification::CRITICAL:   return "!!";
        case Classification::BEST:       return "*";
        case Classification::EXCELLENT:  return "!";
        case Classification::INACCURACY: return "?!";
        case Classification::MISTAKE:    return "?";
        case Classification::BLUNDER:    return "??";
        case Classification::FORCED:     return "[]";
        case Classification::THEORY:     return "T";
        case Classification::RISKY:      return "~";
        default:                         return "";
    }
}

inline const char* classificationName(Classification c) {
    switch (c) {
        case Classification::BRILLIANT:  return "Brilliant";
        case Classification::CRITICAL:   return "Critical";
        case Classification::BEST:       return "Best";
        case Classification::EXCELLENT:  return "Excellent";
        case Classification::OKAY:       return "Okay";
        case Classification::INACCURACY: return "Inaccuracy";
        case Classification::MISTAKE:    return "Mistake";
        case Classification::BLUNDER:    return "Blunder";
        case Classification::FORCED:     return "Forced";
        case Classification::THEORY:     return "Theory";
        case Classification::RISKY:      return "Risky";
        default:                         return "...";
    }
}

inline std::string classificationIconPath(Classification c) {
    std::string base = "img/classifications/";
    switch (c) {
        case Classification::BRILLIANT:  return base + "brilliant.png";
        case Classification::CRITICAL:   return base + "critical.png";
        case Classification::BEST:       return base + "best.png";
        case Classification::EXCELLENT:  return base + "excellent.png";
        case Classification::OKAY:       return base + "okay.png";
        case Classification::INACCURACY: return base + "inaccuracy.png";
        case Classification::MISTAKE:    return base + "mistake.png";
        case Classification::BLUNDER:    return base + "blunder.png";
        case Classification::FORCED:     return base + "forced.png";
        case Classification::THEORY:     return base + "theory.png";
        case Classification::RISKY:      return base + "risky.png";
        default:                         return base + "loading.png";
    }
}

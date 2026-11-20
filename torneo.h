#pragma once
#include <string>

static const int NUM_BOT = 8;
static const int NUM_ROUND = 3;
static const int TEMPERAMENTO_CP = 10;
static const double DURATA_TORNEO_SEC = 6.0 * 3600.0;

static const int PARTITE_MATCH = 3;
static const int PARTITE_FINALE = 3;
static const float BRACKET_PAUSE = 10.0f;
static const float GAME_DELAY_SEC = 0.0f; // pausa tra un game e il prossimo
static const int BRACKET_OGNI = 5;

struct BotTorneo
{
    std::string nome;
    int seed;
};

struct Partita
{
    int idx_bianco;
    int idx_nero;
    int wins_bianco;
    int wins_nero;
    int games_played;
    int vincitore;
    bool giocata;
};

enum StatoTorneo
{
    ST_IN_PARTITA,
    ST_DELAY_GAME, // 5 s tra un game e il prossimo (stesso match)
    ST_MOSTRA_BRACKET,
    ST_FINITO
};

extern BotTorneo bot_lista[NUM_BOT];
extern Partita bracket[NUM_ROUND][8];
extern int partite_per_round[NUM_ROUND];
extern StatoTorneo stato_torneo;
extern int round_corrente;
extern int match_corrente;
extern double tempo_inizio_torneo;
extern float bracket_timer;
extern float game_delay_timer; // ← nuovo
extern int campione_idx;

void inizializza_torneo();
void prepara_game();
void on_game_finito(int colore_perdente);
void disegna_bracket(int sw, int sh);

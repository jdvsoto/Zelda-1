#pragma once
#include <cstring>

// ---- Terminal dimensions ----
#define TERM_COLS    80
#define TERM_ROWS    24

// ---- Room dimensions (including border walls) ----
#define ROOM_W       50
#define ROOM_H       18

// Room screen offset (centers room in 80-col terminal)
#define ROOM_OFF_ROW  3
#define ROOM_OFF_COL  ((TERM_COLS - ROOM_W) / 2)  // 15

// Door positions (room-local coordinates)
#define DOOR_N_COL   (ROOM_W / 2)      // col 25 - top wall
#define DOOR_S_COL   (ROOM_W / 2)      // col 25 - bottom wall
#define DOOR_E_ROW   (ROOM_H / 2)      // row 9  - right wall
#define DOOR_W_ROW   (ROOM_H / 2)      // row 9  - left wall

// Player movement bounds (inner area, excluding walls)
#define PLR_R_MIN    1
#define PLR_R_MAX    (ROOM_H - 2)    // 16
#define PLR_C_MIN    1
#define PLR_C_MAX    (ROOM_W - 2)    // 48

// ---- Game limits ----
#define MAX_ROOMS    4
#define MAX_ENEMIES  6
#define MAX_ITEMS    10
#define MAX_SCORES   10
#define NAME_LEN     12

// ---- Scoring constants ----
#define SCORE_BASIC   10
#define SCORE_CHASER  20
#define SCORE_BOSS   100
#define SCORE_COIN     5
#define SCORE_WIN     50

// ---- Color pair IDs (used with ncurses COLOR_PAIR) ----
#define CP_DEFAULT    1
#define CP_PLAYER     2
#define CP_ENEMY_E    3
#define CP_ENEMY_C    4
#define CP_BOSS       5
#define CP_HEART      6
#define CP_KEY        7
#define CP_COIN       8
#define CP_WALL       9
#define CP_DOOR_OPN  10
#define CP_DOOR_CLO  11
#define CP_HUD       12
#define CP_HUD_HP    13
#define CP_TTL       14
#define CP_SEL       15
#define CP_NORM      16
#define CP_OVER      17
#define CP_WIN       18
#define CP_PROJ      19
#define CP_STAT      20

// ---- Enumerations ----
enum Direction  { DIR_UP = 0, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
enum EnemyType  { ENT_BASIC = 0, ENT_CHASER, ENT_BOSS };
enum ItemType   { ITEM_HEART = 0, ITEM_KEY, ITEM_COIN };

enum Screen {
    SCR_MENU = 0,
    SCR_INSTR,
    SCR_SCORES,
    SCR_MODE,
    SCR_GAME,
    SCR_PAUSE,
    SCR_OVER,
    SCR_WIN,
    SCR_NAME
};

// ---- Structs ----
struct Pos { int r, c; };

struct Enemy {
    Pos       pos;
    EnemyType type;
    int       hp, maxHp;
    bool      alive;
    Direction facing;
};

struct Item {
    Pos      pos;
    ItemType  type;
    bool     collected;
};

struct Room {
    // Door connections: index of connected room, -1 = solid wall
    int  north, south, east, west;
    bool northLock, southLock, eastLock, westLock;

    Enemy enemies[MAX_ENEMIES];
    int   numEnemies;

    Item  items[MAX_ITEMS];
    int   numItems;

    bool  visited;
    char  name[32];
};

struct Player {
    Pos       pos;
    Direction facing;
    int       hp, maxHp;
    int       score;
    int       keys;
};

struct ScoreEntry {
    char name[NAME_LEN + 1];
    int  score;
    int  mode;
};

struct GameState {
    Player  player;
    Room    rooms[MAX_ROOMS];
    int     curRoom;
    int     mode;       // 1 = human, 2 = computer
    Screen  screen;
    bool    running;
    char    msg[80];
    int     msgTtl;     // frames remaining to show msg
};

// ---- Inline helpers ----
inline char playerGlyph(Direction d) {
    switch (d) {
        case DIR_UP:    return '^';
        case DIR_DOWN:  return 'v';
        case DIR_LEFT:  return '<';
        case DIR_RIGHT: return '>';
    }
    return '@';
}

inline char enemyGlyph(EnemyType t) {
    switch (t) {
        case ENT_BASIC:  return 'E';
        case ENT_CHASER: return 'C';
        case ENT_BOSS:   return 'B';
    }
    return '?';
}

inline char itemGlyph(ItemType t) {
    switch (t) {
        case ITEM_HEART: return 'H';
        case ITEM_KEY:   return 'K';
        case ITEM_COIN:  return 'o';
    }
    return '?';
}

inline int enemyColor(EnemyType t) {
    switch (t) {
        case ENT_BASIC:  return CP_ENEMY_E;
        case ENT_CHASER: return CP_ENEMY_C;
        case ENT_BOSS:   return CP_BOSS;
    }
    return CP_DEFAULT;
}

inline int itemColor(ItemType t) {
    switch (t) {
        case ITEM_HEART: return CP_HEART;
        case ITEM_KEY:   return CP_KEY;
        case ITEM_COIN:  return CP_COIN;
    }
    return CP_DEFAULT;
}

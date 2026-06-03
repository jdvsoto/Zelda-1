#include <ncurses.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "game.h"
#include "renderer.h"
#include "scores.h"

// ============================================================
// Room initialization
// ============================================================
static void initRooms(GameState& gs) {
    // --- Room 0: Entrada (NW) ---
    // Connections: East->1, South->2
    {
        Room& r = gs.rooms[0];
        memset(&r, 0, sizeof(r));
        strncpy(r.name, "Entrada del Laberinto", 31);
        r.north = -1;  r.south = 2;  r.east = 1;  r.west = -1;
        r.northLock = false; r.southLock = false;
        r.eastLock  = false; r.westLock  = false;

        r.numEnemies = 2;
        r.enemies[0] = {{5,  10}, ENT_BASIC,  2, 2, true, DIR_DOWN};
        r.enemies[1] = {{12, 38}, ENT_BASIC,  2, 2, true, DIR_LEFT};

        r.numItems = 3;
        r.items[0] = {{3,  22}, ITEM_COIN,  false};
        r.items[1] = {{14, 30}, ITEM_COIN,  false};
        r.items[2] = {{8,  42}, ITEM_HEART, false};
        r.visited = false;
    }

    // --- Room 1: Sala de Llaves (NE) ---
    // Connections: West->0, South->3 (LOCKED)
    {
        Room& r = gs.rooms[1];
        memset(&r, 0, sizeof(r));
        strncpy(r.name, "Sala de las Llaves", 31);
        r.north = -1;  r.south = 3;  r.east = -1;  r.west = 0;
        r.northLock = false; r.southLock = true;
        r.eastLock  = false; r.westLock  = false;

        r.numEnemies = 2;
        r.enemies[0] = {{4,  15}, ENT_BASIC,  2, 2, true, DIR_RIGHT};
        r.enemies[1] = {{11, 35}, ENT_CHASER, 3, 3, true, DIR_UP};

        r.numItems = 3;
        r.items[0] = {{7,  25}, ITEM_KEY,  false};
        r.items[1] = {{3,  38}, ITEM_COIN, false};
        r.items[2] = {{13, 12}, ITEM_COIN, false};
        r.visited = false;
    }

    // --- Room 2: Sala de Guardia (SW) ---
    // Connections: North->0, East->3 (LOCKED)
    {
        Room& r = gs.rooms[2];
        memset(&r, 0, sizeof(r));
        strncpy(r.name, "Sala de Guardia", 31);
        r.north = 0;  r.south = -1;  r.east = 3;  r.west = -1;
        r.northLock = false; r.southLock = false;
        r.eastLock  = true;  r.westLock  = false;

        r.numEnemies = 3;
        r.enemies[0] = {{3,  8},  ENT_CHASER, 3, 3, true, DIR_DOWN};
        r.enemies[1] = {{13, 40}, ENT_CHASER, 3, 3, true, DIR_LEFT};
        r.enemies[2] = {{8,  24}, ENT_BASIC,  2, 2, true, DIR_UP};

        r.numItems = 3;
        r.items[0] = {{5,  42}, ITEM_KEY,   false};
        r.items[1] = {{12, 10}, ITEM_HEART, false};
        r.items[2] = {{9,  30}, ITEM_COIN,  false};
        r.visited = false;
    }

    // --- Room 3: Sala del Jefe (SE) ---
    // Connections: North->1 (LOCKED), West->2 (LOCKED)
    {
        Room& r = gs.rooms[3];
        memset(&r, 0, sizeof(r));
        strncpy(r.name, "Sala del Jefe  [BOSS]", 31);
        r.north = 1;  r.south = -1;  r.east = -1;  r.west = 2;
        r.northLock = true; r.southLock = false;
        r.eastLock  = false; r.westLock  = true;

        r.numEnemies = 1;
        r.enemies[0] = {{ROOM_H/2, ROOM_W/2}, ENT_BOSS, 10, 10, true, DIR_DOWN};

        r.numItems = 1;
        r.items[0] = {{3, 40}, ITEM_HEART, false};
        r.visited = false;
    }
}

// ============================================================
// Game initialization
// ============================================================
static void initGame(GameState& gs, int mode) {
    memset(&gs, 0, sizeof(gs));
    gs.mode    = mode;
    gs.screen  = SCR_GAME;
    gs.running = true;
    gs.curRoom = 0;

    gs.player.pos    = {ROOM_H / 2, ROOM_W / 2};
    gs.player.facing = DIR_DOWN;
    gs.player.hp     = 3;
    gs.player.maxHp  = 5;
    gs.player.score  = 0;
    gs.player.keys   = 0;

    initRooms(gs);
    gs.rooms[0].visited = true;

    strncpy(gs.msg, "Bienvenido! Derrota al Jefe para ganar. P=Pausa, Z=Atacar.", 79);
    gs.msgTtl = 150;
}

// ============================================================
// Room transition
// ============================================================
static void tryTransition(GameState& gs, Direction dir) {
    Room& room = gs.rooms[gs.curRoom];
    int  dest  = -1;
    bool locked = false;

    switch (dir) {
        case DIR_UP:    dest = room.north; locked = room.northLock; break;
        case DIR_DOWN:  dest = room.south; locked = room.southLock; break;
        case DIR_LEFT:  dest = room.west;  locked = room.westLock;  break;
        case DIR_RIGHT: dest = room.east;  locked = room.eastLock;  break;
    }

    if (dest == -1) return;  // solid wall

    if (locked) {
        if (gs.player.keys > 0) {
            gs.player.keys--;
            // Unlock in both directions
            switch (dir) {
                case DIR_UP:
                    room.northLock = false;
                    gs.rooms[dest].southLock = false;
                    break;
                case DIR_DOWN:
                    room.southLock = false;
                    gs.rooms[dest].northLock = false;
                    break;
                case DIR_LEFT:
                    room.westLock = false;
                    gs.rooms[dest].eastLock  = false;
                    break;
                case DIR_RIGHT:
                    room.eastLock = false;
                    gs.rooms[dest].westLock  = false;
                    break;
            }
            strncpy(gs.msg, "Puerta desbloqueada con la llave!", 79);
            gs.msgTtl = 80;
        } else {
            strncpy(gs.msg, "Esta puerta esta cerrada. Necesitas una llave (K).", 79);
            gs.msgTtl = 90;
            return;
        }
    }

    gs.curRoom = dest;
    gs.rooms[dest].visited = true;

    // Place player at opposite side of destination room
    switch (dir) {
        case DIR_UP:    gs.player.pos = {PLR_R_MAX - 1, DOOR_S_COL}; break;
        case DIR_DOWN:  gs.player.pos = {PLR_R_MIN + 1, DOOR_N_COL}; break;
        case DIR_LEFT:  gs.player.pos = {DOOR_E_ROW,    PLR_C_MAX - 1}; break;
        case DIR_RIGHT: gs.player.pos = {DOOR_W_ROW,    PLR_C_MIN + 1}; break;
    }

    char tmp[80];
    snprintf(tmp, 79, "Entrando a: %s", gs.rooms[dest].name);
    strncpy(gs.msg, tmp, 79);
    gs.msgTtl = 60;
}

// ============================================================
// Player movement
// ============================================================
static void movePlayer(GameState& gs, Direction dir) {
    Player& p = gs.player;
    p.facing = dir;

    int nr = p.pos.r, nc = p.pos.c;
    switch (dir) {
        case DIR_UP:    nr--; break;
        case DIR_DOWN:  nr++; break;
        case DIR_LEFT:  nc--; break;
        case DIR_RIGHT: nc++; break;
    }

    // Check door transitions (step into door cell)
    if (nr == 0            && nc == DOOR_N_COL) { tryTransition(gs, DIR_UP);    return; }
    if (nr == ROOM_H - 1   && nc == DOOR_S_COL) { tryTransition(gs, DIR_DOWN);  return; }
    if (nc == ROOM_W - 1   && nr == DOOR_E_ROW) { tryTransition(gs, DIR_RIGHT); return; }
    if (nc == 0            && nr == DOOR_W_ROW) { tryTransition(gs, DIR_LEFT);  return; }

    // Wall bounds
    if (nr < PLR_R_MIN || nr > PLR_R_MAX ||
        nc < PLR_C_MIN || nc > PLR_C_MAX) return;

    p.pos.r = nr;
    p.pos.c = nc;

    // Item pickup
    Room& room = gs.rooms[gs.curRoom];
    for (int i = 0; i < room.numItems; i++) {
        Item& item = room.items[i];
        if (!item.collected && item.pos.r == p.pos.r && item.pos.c == p.pos.c) {
            item.collected = true;
            char tmp[80];
            switch (item.type) {
                case ITEM_HEART:
                    if (p.hp < p.maxHp) { p.hp++; snprintf(tmp, 79, "Corazon recogido! Vida: %d/%d", p.hp, p.maxHp); }
                    else                  snprintf(tmp, 79, "Vida ya maxima, pero recoges el corazon.");
                    break;
                case ITEM_KEY:
                    p.keys++;
                    snprintf(tmp, 79, "Llave recogida! Llaves: %d", p.keys);
                    break;
                case ITEM_COIN:
                    p.score += SCORE_COIN;
                    snprintf(tmp, 79, "Rupia recogida! +%d pts  Total: %d", SCORE_COIN, p.score);
                    break;
            }
            strncpy(gs.msg, tmp, 79);
            gs.msgTtl = 60;
        }
    }

    // Enemy collision (take damage)
    for (int i = 0; i < room.numEnemies; i++) {
        Enemy& e = room.enemies[i];
        if (!e.alive) continue;
        if (e.pos.r == p.pos.r && e.pos.c == p.pos.c) {
            p.hp--;
            // Bounce back
            p.pos.r -= (dir == DIR_DOWN ? 1 : (dir == DIR_UP    ? -1 : 0));
            p.pos.c -= (dir == DIR_RIGHT? 1 : (dir == DIR_LEFT   ? -1 : 0));

            char tmp[80];
            snprintf(tmp, 79, "Golpeado! Vida: %d/%d", p.hp, p.maxHp);
            strncpy(gs.msg, tmp, 79);
            gs.msgTtl = 70;

            if (p.hp <= 0) gs.screen = SCR_OVER;
            break;
        }
    }
}

// ============================================================
// Player attack
// ============================================================
static void playerAttack(GameState& gs) {
    Player& p = gs.player;
    Room&   room = gs.rooms[gs.curRoom];

    // Attack cell = one step in facing direction
    int ar = p.pos.r, ac = p.pos.c;
    switch (p.facing) {
        case DIR_UP:    ar--; break;
        case DIR_DOWN:  ar++; break;
        case DIR_LEFT:  ac--; break;
        case DIR_RIGHT: ac++; break;
    }

    bool hit = false;
    for (int i = 0; i < room.numEnemies; i++) {
        Enemy& e = room.enemies[i];
        if (!e.alive) continue;

        int dr = e.pos.r - ar, dc = e.pos.c - ac;
        // Hit if enemy is at attack cell or adjacent
        if (dr * dr + dc * dc <= 1) {
            e.hp--;
            hit = true;
            if (e.hp <= 0) {
                e.alive = false;
                int pts = (e.type == ENT_BASIC)  ? SCORE_BASIC  :
                          (e.type == ENT_CHASER) ? SCORE_CHASER : SCORE_BOSS;
                p.score += pts;

                if (e.type == ENT_BOSS) {
                    p.score += SCORE_WIN;
                    gs.screen = SCR_WIN;
                    strncpy(gs.msg, "VICTORIA! El Jefe ha sido derrotado!", 79);
                    gs.msgTtl = 9999;
                } else {
                    char tmp[80];
                    snprintf(tmp, 79, "Enemigo derrotado! +%d pts  Total: %d", pts, p.score);
                    strncpy(gs.msg, tmp, 79);
                    gs.msgTtl = 60;
                }
            } else {
                char tmp[80];
                snprintf(tmp, 79, "Golpe al enemigo! HP enemigo: %d/%d", e.hp, e.maxHp);
                strncpy(gs.msg, tmp, 79);
                gs.msgTtl = 40;
            }
            break;
        }
    }

    if (!hit) {
        strncpy(gs.msg, "Ataque con la espada! (no hay enemigo cercano)", 79);
        gs.msgTtl = 25;
    }
}

// ============================================================
// AI controller (Mode 2 - very basic for Phase 2)
// ============================================================
static int aiTimer = 0;

static void updateAI(GameState& gs) {
    aiTimer++;
    if (aiTimer < 8) return;  // move every 8 frames
    aiTimer = 0;

    const Player& p = gs.player;
    const Room&   room = gs.rooms[gs.curRoom];

    // Find nearest alive enemy
    int    bestDist = 999999;
    int    bestIdx  = -1;
    for (int i = 0; i < room.numEnemies; i++) {
        if (!room.enemies[i].alive) continue;
        int dr = room.enemies[i].pos.r - p.pos.r;
        int dc = room.enemies[i].pos.c - p.pos.c;
        int dist = dr * dr + dc * dc;
        if (dist < bestDist) { bestDist = dist; bestIdx = i; }
    }

    if (bestIdx == -1) {
        // No enemies, go to next open door
        if (room.east != -1 && !room.eastLock) movePlayer(gs, DIR_RIGHT);
        else if (room.south != -1 && !room.southLock) movePlayer(gs, DIR_DOWN);
        else if (room.north != -1 && !room.northLock) movePlayer(gs, DIR_UP);
        else if (room.west != -1  && !room.westLock)  movePlayer(gs, DIR_LEFT);
        return;
    }

    const Enemy& tgt = room.enemies[bestIdx];
    int dr = tgt.pos.r - p.pos.r;
    int dc = tgt.pos.c - p.pos.c;

    // Attack if adjacent
    if (dr * dr + dc * dc <= 2) {
        playerAttack(gs);
        return;
    }

    // Move toward enemy using Manhattan heuristic
    if (abs(dr) >= abs(dc)) {
        movePlayer(gs, dr > 0 ? DIR_DOWN : DIR_UP);
    } else {
        movePlayer(gs, dc > 0 ? DIR_RIGHT : DIR_LEFT);
    }
}

// ============================================================
// Input handlers per screen
// ============================================================
static int  menuSel   = 0;
static int  modeSel   = 0;
static int  instrPage = 0;
static ScoreEntry scores[MAX_SCORES];
static int  numScores = 0;
static char nameInput[NAME_LEN + 1];
static int  nameCursor = 0;

static void handleMenu(int ch, GameState& gs) {
    const int N = 4;
    switch (ch) {
        case KEY_UP:   case 'w': case 'W':
            menuSel = (menuSel - 1 + N) % N; break;
        case KEY_DOWN: case 's': case 'S':
            menuSel = (menuSel + 1) % N; break;
        case '\n': case '\r': case ' ':
            switch (menuSel) {
                case 0: gs.screen = SCR_MODE;   modeSel = 0; break;
                case 1: gs.screen = SCR_INSTR;  instrPage = 0; break;
                case 2: gs.screen = SCR_SCORES; loadScores(scores, numScores); break;
                case 3: gs.running = false; break;
            }
            break;
        case 'q': case 'Q': case 27:
            gs.running = false; break;
    }
}

static void handleMode(int ch, GameState& gs) {
    switch (ch) {
        case KEY_LEFT:  case KEY_RIGHT:
        case 'a': case 'A': case 'd': case 'D':
            modeSel = 1 - modeSel; break;
        case '\n': case '\r': case ' ':
            initGame(gs, modeSel + 1); break;
        case 27: case 'q': case 'Q':
            gs.screen = SCR_MENU; break;
    }
}

static void handleInstructions(int ch, GameState& gs) {
    const int MAX_PAGE = 2;
    switch (ch) {
        case KEY_RIGHT: case 'n': case 'N':
            if (instrPage < MAX_PAGE) { instrPage++; } break;
        case KEY_LEFT: case 'b': case 'B':
            if (instrPage > 0) { instrPage--; } break;
        case 27: case 'q': case 'Q': case '\n': case '\r':
            gs.screen = SCR_MENU; break;
    }
}

static void handleScores(int ch, GameState& gs) {
    if (ch == 27 || ch == 'q' || ch == 'Q' || ch == '\n' || ch == '\r') {
        gs.screen = SCR_MENU;
    }
}

static void handleGame(int ch, GameState& gs) {
    switch (ch) {
        case KEY_UP:    case 'w': case 'W': movePlayer(gs, DIR_UP);    break;
        case KEY_DOWN:  case 's': case 'S': movePlayer(gs, DIR_DOWN);  break;
        case KEY_LEFT:  case 'a': case 'A': movePlayer(gs, DIR_LEFT);  break;
        case KEY_RIGHT: case 'd': case 'D': movePlayer(gs, DIR_RIGHT); break;
        case ' ':       case 'z': case 'Z': playerAttack(gs);          break;
        case 'p': case 'P':                 gs.screen = SCR_PAUSE;     break;
        case 'q': case 'Q': case 27:        gs.screen = SCR_MENU;      break;
    }
}

static void handlePause(int ch, GameState& gs) {
    switch (ch) {
        case 'p': case 'P': case '\n': case '\r': case ' ':
            gs.screen = SCR_GAME; break;
        case 'q': case 'Q': case 27:
            gs.screen = SCR_MENU; break;
    }
}

static void handleOver(int ch, GameState& gs) {
    switch (ch) {
        case 'r': case 'R':
            gs.screen = SCR_MODE; modeSel = gs.mode - 1; break;
        case '\n': case '\r': case ' ':
            memset(nameInput, 0, sizeof(nameInput));
            nameCursor = 0;
            gs.screen = SCR_NAME;
            break;
        case 'q': case 'Q': case 27:
            gs.screen = SCR_MENU; break;
    }
}

static void handleWin(int ch, GameState& gs) {
    switch (ch) {
        case 'r': case 'R':
            gs.screen = SCR_MODE; modeSel = gs.mode - 1; break;
        case '\n': case '\r': case ' ':
            memset(nameInput, 0, sizeof(nameInput));
            nameCursor = 0;
            gs.screen = SCR_NAME;
            break;
        case 'q': case 'Q': case 27:
            gs.screen = SCR_MENU; break;
    }
}

static void handleName(int ch, GameState& gs) {
    if (ch == '\n' || ch == '\r') {
        if (nameCursor > 0) {
            nameInput[nameCursor] = '\0';
            addScore(scores, numScores, nameInput, gs.player.score, gs.mode);
            saveScores(scores, numScores);
        }
        gs.screen = SCR_SCORES;
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
        if (nameCursor > 0) { nameInput[--nameCursor] = '\0'; }
    } else if (ch >= 32 && ch < 127 && nameCursor < NAME_LEN) {
        nameInput[nameCursor++] = (char)ch;
        nameInput[nameCursor]   = '\0';
    } else if (ch == 27) {
        gs.screen = SCR_MENU;
    }
}

// ============================================================
// Main
// ============================================================
int main() {
    rendererInit();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    if (rows < TERM_ROWS || cols < TERM_COLS) {
        endwin();
        printf("Terminal demasiado pequena!\n");
        printf("Necesitas al menos %dx%d  (actual: %dx%d)\n",
               TERM_COLS, TERM_ROWS, cols, rows);
        return 1;
    }

    loadScores(scores, numScores);

    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.screen  = SCR_MENU;
    gs.running = true;
    menuSel    = 0;

    while (gs.running) {
        int ch = getch();

        // Dispatch input
        switch (gs.screen) {
            case SCR_MENU:   handleMenu(ch, gs);         break;
            case SCR_MODE:   handleMode(ch, gs);         break;
            case SCR_INSTR:  handleInstructions(ch, gs); break;
            case SCR_SCORES: handleScores(ch, gs);       break;
            case SCR_GAME:
                if (gs.mode == 2) updateAI(gs);
                else              handleGame(ch, gs);
                // P always pauses even in mode 2
                if (ch == 'p' || ch == 'P') gs.screen = SCR_PAUSE;
                if (ch == 'q' || ch == 'Q' || ch == 27) gs.screen = SCR_MENU;
                break;
            case SCR_PAUSE:  handlePause(ch, gs);   break;
            case SCR_OVER:   handleOver(ch, gs);    break;
            case SCR_WIN:    handleWin(ch, gs);     break;
            case SCR_NAME:   handleName(ch, gs);    break;
        }

        if (gs.msgTtl > 0) gs.msgTtl--;

        // Render
        clear();
        switch (gs.screen) {
            case SCR_MENU:   drawMainMenu(menuSel);        break;
            case SCR_MODE:   drawModeSelect(modeSel);      break;
            case SCR_INSTR:  drawInstructions(instrPage);  break;
            case SCR_SCORES: drawScores(scores, numScores); break;
            case SCR_GAME:   drawGame(gs);                 break;
            case SCR_PAUSE:  drawGame(gs); drawPause();    break;
            case SCR_OVER:   drawGameOver(gs);             break;
            case SCR_WIN:    drawVictory(gs);              break;
            case SCR_NAME:   drawEnterName(nameInput, nameCursor); break;
        }
        refresh();

        usleep(50000);  // ~20 FPS
    }

    rendererCleanup();
    return 0;
}

#include "renderer.h"
#include "game.h"
#include <ncurses.h>
#include <cstring>
#include <cstdio>

// ============================================================
// Initialization / Cleanup
// ============================================================
void rendererInit() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        init_pair(CP_DEFAULT,  COLOR_WHITE,   COLOR_BLACK);
        init_pair(CP_PLAYER,   COLOR_WHITE,   COLOR_BLACK);
        init_pair(CP_ENEMY_E,  COLOR_YELLOW,  COLOR_BLACK);
        init_pair(CP_ENEMY_C,  COLOR_RED,     COLOR_BLACK);
        init_pair(CP_BOSS,     COLOR_RED,     COLOR_BLACK);
        init_pair(CP_HEART,    COLOR_RED,     COLOR_BLACK);
        init_pair(CP_KEY,      COLOR_YELLOW,  COLOR_BLACK);
        init_pair(CP_COIN,     COLOR_GREEN,   COLOR_BLACK);
        init_pair(CP_WALL,     COLOR_CYAN,    COLOR_BLACK);
        init_pair(CP_DOOR_OPN, COLOR_GREEN,   COLOR_BLACK);
        init_pair(CP_DOOR_CLO, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(CP_HUD,      COLOR_WHITE,   COLOR_BLUE);
        init_pair(CP_HUD_HP,   COLOR_RED,     COLOR_BLUE);
        init_pair(CP_TTL,      COLOR_YELLOW,  COLOR_BLACK);
        init_pair(CP_SEL,      COLOR_BLACK,   COLOR_WHITE);
        init_pair(CP_NORM,     COLOR_WHITE,   COLOR_BLACK);
        init_pair(CP_OVER,     COLOR_RED,     COLOR_BLACK);
        init_pair(CP_WIN,      COLOR_YELLOW,  COLOR_BLACK);
        init_pair(CP_PROJ,     COLOR_WHITE,   COLOR_BLACK);
        init_pair(CP_STAT,     COLOR_CYAN,    COLOR_BLACK);
    }
}

void rendererCleanup() {
    endwin();
}

// ============================================================
// Internal helpers
// ============================================================

// Print a centered string at given row
static void printCenter(int row, const char* str, int attrs) {
    int len = (int)strlen(str);
    int col = (TERM_COLS - len) / 2;
    if (col < 0) col = 0;
    attron(attrs);
    mvprintw(row, col, "%s", str);
    attroff(attrs);
}

// Horizontal separator bar
static void hline80(int row, chtype ch = ACS_HLINE) {
    move(row, 0);
    for (int i = 0; i < TERM_COLS; i++) addch(ch);
}

// Box (using '+', '-', '|' for portability)
static void drawBox(int r, int c, int h, int w, int attrs) {
    attron(attrs);
    mvaddch(r,       c,       '+');
    mvaddch(r,       c+w-1,   '+');
    mvaddch(r+h-1,   c,       '+');
    mvaddch(r+h-1,   c+w-1,   '+');
    for (int i = 1; i < w-1; i++) {
        mvaddch(r,     c+i, '-');
        mvaddch(r+h-1, c+i, '-');
    }
    for (int i = 1; i < h-1; i++) {
        mvaddch(r+i, c,     '|');
        mvaddch(r+i, c+w-1, '|');
    }
    attroff(attrs);
}

// ============================================================
// Main Menu
// ============================================================
void drawMainMenu(int selected) {
    const char* ITEMS[] = {
        "  NUEVA PARTIDA  ",
        "  INSTRUCCIONES  ",
        "  PUNTAJES       ",
        "  SALIR          "
    };
    const int N = 4;

    // Background fill
    bkgd(COLOR_PAIR(CP_DEFAULT));

    // ASCII art title block
    attron(COLOR_PAIR(CP_TTL) | A_BOLD);
    printCenter(2, "+-----------------------------------------+", 0);
    printCenter(3, "|                                         |", 0);
    printCenter(4, "|   THE LEGEND OF ZELDA - ASCII EDITION  |", 0);
    printCenter(5, "|                                         |", 0);
    printCenter(6, "+-----------------------------------------+", 0);
    attroff(COLOR_PAIR(CP_TTL) | A_BOLD);

    // Triforce-style decoration
    attron(COLOR_PAIR(CP_COIN) | A_BOLD);
    printCenter(7,  "            /\\         ", 0);
    printCenter(8,  "           /  \\        ", 0);
    printCenter(9,  "          /    \\       ", 0);
    printCenter(10, "         /      \\      ", 0);
    printCenter(11, "         ---------     ", 0);
    printCenter(12, "       /\\         /\\   ", 0);
    printCenter(13, "      /  \\       /  \\  ", 0);
    printCenter(14, "     /    \\     /    \\ ", 0);
    printCenter(15, "    /      \\   /      \\", 0);
    printCenter(16, "    --------- ---------", 0);

    attroff(COLOR_PAIR(CP_COIN) | A_BOLD);

    // Menu items
    int menuStartRow = 18;
    int menuCol = (TERM_COLS - 22) / 2;

    for (int i = 0; i < N; i++) {
        int row = menuStartRow + i;
        if (i == selected) {
            attron(COLOR_PAIR(CP_SEL) | A_BOLD);
            mvprintw(row, menuCol, "> %s <", ITEMS[i]);
            attroff(COLOR_PAIR(CP_SEL) | A_BOLD);
        } else {
            attron(COLOR_PAIR(CP_NORM));
            mvprintw(row, menuCol, "  %s  ", ITEMS[i]);
            attroff(COLOR_PAIR(CP_NORM));
        }
    }

    // Footer hint
    attron(COLOR_PAIR(CP_STAT));
    printCenter(22, "W/S o Flechas: Navegar  |  Enter: Seleccionar  |  Q: Salir", 0);
    attroff(COLOR_PAIR(CP_STAT));

    attron(COLOR_PAIR(CP_DEFAULT) | A_DIM);
    printCenter(23, "CC3086 - Programacion de Microprocesadores - UVG 2026", 0);
    attroff(COLOR_PAIR(CP_DEFAULT) | A_DIM);
}

// ============================================================
// Mode Select
// ============================================================
void drawModeSelect(int selected) {
    bkgd(COLOR_PAIR(CP_DEFAULT));

    attron(COLOR_PAIR(CP_TTL) | A_BOLD);
    printCenter(3, "SELECCIONAR MODO DE JUEGO", 0);
    attroff(COLOR_PAIR(CP_TTL) | A_BOLD);

    hline80(4);

    // Mode 1
    int col1 = 5, col2 = 42, row = 7;
    int boxW = 32, boxH = 9;

    // Mode 1 box
    drawBox(row, col1, boxH, boxW,
            selected == 0 ? COLOR_PAIR(CP_WIN) | A_BOLD : COLOR_PAIR(CP_NORM));

    attron(selected == 0 ? COLOR_PAIR(CP_WIN) | A_BOLD : COLOR_PAIR(CP_NORM));
    mvprintw(row+1, col1+2, "MODO 1: UN JUGADOR");
    mvprintw(row+3, col1+2, "Controla a Link con el");
    mvprintw(row+4, col1+2, "teclado.");
    mvprintw(row+6, col1+2, "W/A/S/D o flechas");
    mvprintw(row+7, col1+2, "Espacio/Z: Atacar");
    attroff(selected == 0 ? COLOR_PAIR(CP_WIN) | A_BOLD : COLOR_PAIR(CP_NORM));

    // Mode 2 box
    drawBox(row, col2, boxH, boxW,
            selected == 1 ? COLOR_PAIR(CP_WIN) | A_BOLD : COLOR_PAIR(CP_NORM));

    attron(selected == 1 ? COLOR_PAIR(CP_WIN) | A_BOLD : COLOR_PAIR(CP_NORM));
    mvprintw(row+1, col2+2, "MODO 2: COMPUTADORA");
    mvprintw(row+3, col2+2, "La IA controla a Link");
    mvprintw(row+4, col2+2, "automaticamente.");
    mvprintw(row+6, col2+2, "Observa como juega");
    mvprintw(row+7, col2+2, "la computadora.");
    attroff(selected == 1 ? COLOR_PAIR(CP_WIN) | A_BOLD : COLOR_PAIR(CP_NORM));

    // Selection arrow
    if (selected == 0) {
        attron(COLOR_PAIR(CP_TTL) | A_BOLD);
        mvprintw(row + boxH/2, col1 - 3, ">>>");
        attroff(COLOR_PAIR(CP_TTL) | A_BOLD);
    } else {
        attron(COLOR_PAIR(CP_TTL) | A_BOLD);
        mvprintw(row + boxH/2, col2 - 3, ">>>");
        attroff(COLOR_PAIR(CP_TTL) | A_BOLD);
    }

    attron(COLOR_PAIR(CP_STAT));
    printCenter(20, "A/D o Flechas: Cambiar modo  |  Enter: Confirmar  |  Q: Volver", 0);
    attroff(COLOR_PAIR(CP_STAT));
}

// ============================================================
// Instructions (3 pages)
// ============================================================
void drawInstructions(int page) {
    bkgd(COLOR_PAIR(CP_DEFAULT));

    attron(COLOR_PAIR(CP_TTL) | A_BOLD);
    printCenter(1, "INSTRUCCIONES DEL JUEGO", 0);
    attroff(COLOR_PAIR(CP_TTL) | A_BOLD);
    hline80(2);

    if (page == 0) {
        attron(COLOR_PAIR(CP_WIN) | A_BOLD);
        mvprintw(4, 4, "OBJETIVO:");
        attroff(COLOR_PAIR(CP_WIN) | A_BOLD);
        attron(COLOR_PAIR(CP_NORM));
        mvprintw(5,  4, "Explora 4 habitaciones interconectadas, derrota a los enemigos,");
        mvprintw(6,  4, "recoge llaves para abrir puertas cerradas y vence al Jefe (B)");
        mvprintw(7,  4, "en la habitacion final para ganar.");
        attroff(COLOR_PAIR(CP_NORM));

        attron(COLOR_PAIR(CP_WIN) | A_BOLD);
        mvprintw(9, 4, "CONTROLES:");
        attroff(COLOR_PAIR(CP_WIN) | A_BOLD);
        attron(COLOR_PAIR(CP_NORM));
        mvprintw(10, 4,  "  W / Flecha Arriba  : Mover hacia arriba");
        mvprintw(11, 4,  "  S / Flecha Abajo   : Mover hacia abajo");
        mvprintw(12, 4,  "  A / Flecha Izq.    : Mover hacia la izquierda");
        mvprintw(13, 4,  "  D / Flecha Der.    : Mover hacia la derecha");
        mvprintw(14, 4,  "  Espacio / Z        : Atacar con la espada");
        mvprintw(15, 4,  "  P                  : Pausar / Reanudar");
        mvprintw(16, 4,  "  Q / ESC            : Salir / Menu principal");
        attroff(COLOR_PAIR(CP_NORM));
    }
    else if (page == 1) {
        attron(COLOR_PAIR(CP_WIN) | A_BOLD);
        mvprintw(4, 4, "ELEMENTOS DEL JUEGO:");
        attroff(COLOR_PAIR(CP_WIN) | A_BOLD);

        struct { const char* sym; const char* desc; int cp; bool bold; } elems[] = {
            { "^  v  <  >", "Link (segun direccion)",      CP_PLAYER,   true  },
            { "E",          "Enemigo Basico  (+10 pts)",    CP_ENEMY_E,  false },
            { "C",          "Enemigo Perseguidor (+20 pts)",CP_ENEMY_C,  false },
            { "B",          "Jefe Final (+100 pts)",        CP_BOSS,     true  },
            { "#",          "Pared / Muro",                  CP_WALL,     false },
            { "_",          "Puerta abierta",                CP_DOOR_OPN, false },
            { "D",          "Puerta cerrada (necesita llave)",CP_DOOR_CLO,false },
            { "H",          "Corazon (restaura 1 vida)",    CP_HEART,    false },
            { "K",          "Llave (abre puertas)",         CP_KEY,      false },
            { "o",          "Moneda / Rupia (+5 pts)",      CP_COIN,     false },
            { ".",          "Celda transitable vacia",       CP_DEFAULT,  false },
        };
        int n = (int)(sizeof(elems)/sizeof(elems[0]));
        for (int i = 0; i < n; i++) {
            attron(COLOR_PAIR(elems[i].cp) | (elems[i].bold ? A_BOLD : 0));
            mvprintw(5 + i, 6, "  %s", elems[i].sym);
            attroff(COLOR_PAIR(elems[i].cp) | (elems[i].bold ? A_BOLD : 0));
            attron(COLOR_PAIR(CP_NORM));
            mvprintw(5 + i, 22, "- %s", elems[i].desc);
            attroff(COLOR_PAIR(CP_NORM));
        }
    }
    else if (page == 2) {
        attron(COLOR_PAIR(CP_WIN) | A_BOLD);
        mvprintw(4,  4, "SISTEMA DE PUNTUACION:");
        attroff(COLOR_PAIR(CP_WIN) | A_BOLD);
        attron(COLOR_PAIR(CP_NORM));
        mvprintw(5,  4, "  Derrotar Enemigo Basico     :  +10 puntos");
        mvprintw(6,  4, "  Derrotar Enemigo Perseguidor:  +20 puntos");
        mvprintw(7,  4, "  Derrotar al Jefe            : +100 puntos");
        mvprintw(8,  4, "  Recolectar moneda/rupia     :   +5 puntos");
        mvprintw(9,  4, "  Completar nivel             :  +50 puntos");
        attroff(COLOR_PAIR(CP_NORM));

        attron(COLOR_PAIR(CP_WIN) | A_BOLD);
        mvprintw(11, 4, "SISTEMA DE VIDA:");
        attroff(COLOR_PAIR(CP_WIN) | A_BOLD);
        attron(COLOR_PAIR(CP_NORM));
        mvprintw(12, 4, "  Link comienza con 3 corazones (maximo 5).");
        mvprintw(13, 4, "  Recibir un golpe de enemigo: -1 corazon.");
        mvprintw(14, 4, "  Al llegar a 0 corazones: GAME OVER.");
        attroff(COLOR_PAIR(CP_NORM));

        attron(COLOR_PAIR(CP_WIN) | A_BOLD);
        mvprintw(16, 4, "MAPA DE HABITACIONES:");
        attroff(COLOR_PAIR(CP_WIN) | A_BOLD);
        attron(COLOR_PAIR(CP_STAT));
        mvprintw(17, 4, "  [Entrada]---[Sala de Llaves]");
        mvprintw(18, 4, "      |               |");
        mvprintw(19, 4, "  [Guardia]---[Sala del Jefe]  <- BOSS!");
        mvprintw(20, 4, "  (las puertas con 'D' requieren llave)");
        attroff(COLOR_PAIR(CP_STAT));
    }

    // Page indicator and navigation
    hline80(21);
    attron(COLOR_PAIR(CP_STAT));
    char pageStr[32];
    snprintf(pageStr, 31, "Pagina %d/3", page + 1);
    mvprintw(22, 2, "%s", pageStr);
    printCenter(22, "N/Flecha Der.: Sig. | B/Flecha Izq.: Ant. | Q/Enter: Volver", 0);
    attroff(COLOR_PAIR(CP_STAT));
}

// ============================================================
// High Scores
// ============================================================
void drawScores(const ScoreEntry entries[], int count) {
    bkgd(COLOR_PAIR(CP_DEFAULT));

    attron(COLOR_PAIR(CP_TTL) | A_BOLD);
    printCenter(1, "PUNTAJES DESTACADOS", 0);
    attroff(COLOR_PAIR(CP_TTL) | A_BOLD);
    hline80(2);

    // Table header
    int startCol = (TERM_COLS - 50) / 2;
    attron(COLOR_PAIR(CP_WIN) | A_BOLD);
    mvprintw(4, startCol, "%-4s %-12s %8s %6s %8s",
             "#", "NOMBRE", "PUNTOS", "MODO", "MEDALLA");
    attroff(COLOR_PAIR(CP_WIN) | A_BOLD);
    attron(COLOR_PAIR(CP_WALL));
    mvprintw(5, startCol, "%-4s %-12s %8s %6s %8s",
             "----", "------------", "--------", "------", "--------");
    attroff(COLOR_PAIR(CP_WALL));

    if (count == 0) {
        attron(COLOR_PAIR(CP_DEFAULT) | A_DIM);
        printCenter(10, "(Sin puntajes registrados aun)", 0);
        attroff(COLOR_PAIR(CP_DEFAULT) | A_DIM);
    } else {
        const char* medals[] = { "[ORO]", "[PLATA]", "[BRONCE]", "", "" };
        int medalColors[] = { CP_WIN, CP_WALL, CP_COIN, CP_DEFAULT, CP_DEFAULT };

        for (int i = 0; i < count && i < MAX_SCORES; i++) {
            int row = 6 + i;
            int cp = (i < 3) ? medalColors[i] : CP_NORM;
            attron(COLOR_PAIR(cp) | (i < 3 ? A_BOLD : 0));
            mvprintw(row, startCol, "%-4d %-12s %8d  Modo%d  %s",
                     i + 1,
                     entries[i].name,
                     entries[i].score,
                     entries[i].mode,
                     i < 3 ? medals[i] : "");
            attroff(COLOR_PAIR(cp) | (i < 3 ? A_BOLD : 0));
        }
    }

    hline80(21);
    attron(COLOR_PAIR(CP_STAT));
    printCenter(22, "Enter / Q: Volver al menu", 0);
    attroff(COLOR_PAIR(CP_STAT));
}

// ============================================================
// HUD (Heads-Up Display) - top 3 rows
// ============================================================
static void drawHUD(const GameState& gs) {
    const Player& p = gs.player;

    // Top border
    attron(COLOR_PAIR(CP_HUD) | A_BOLD);
    for (int c = 0; c < TERM_COLS; c++) mvaddch(0, c, ' ');

    // Life display
    mvprintw(0, 1, "VIDA:");
    for (int i = 0; i < p.maxHp; i++) {
        if (i < p.hp) {
            attron(COLOR_PAIR(CP_HUD_HP) | A_BOLD);
            mvaddch(0, 7 + i*2, 'H');
            attroff(COLOR_PAIR(CP_HUD_HP) | A_BOLD);
            attron(COLOR_PAIR(CP_HUD) | A_BOLD);
        } else {
            mvaddch(0, 7 + i*2, '-');
        }
    }

    // Score
    attron(COLOR_PAIR(CP_HUD) | A_BOLD);
    mvprintw(0, 20, "PUNTOS:%05d", p.score);

    // Keys
    mvprintw(0, 34, "LLAVES:%d", p.keys);

    // Room info
    mvprintw(0, 44, "HAB:%d/4", gs.curRoom + 1);

    // Mode
    const char* modeStr = (gs.mode == 1) ? "MODO:JUGADOR" : "MODO:CPU    ";
    mvprintw(0, 54, "%s", modeStr);

    // Room name
    for (int c = 0; c < TERM_COLS; c++) mvaddch(1, c, ' ');
    mvprintw(1, 2, "Habitacion: %s", gs.rooms[gs.curRoom].name);
    for (int c = 0; c < TERM_COLS; c++) mvaddch(2, c, ACS_HLINE);

    attroff(COLOR_PAIR(CP_HUD) | A_BOLD);
}

// ============================================================
// Room drawing
// ============================================================
static void drawRoom(const GameState& gs) {
    const Room& room = gs.rooms[gs.curRoom];

    // For each cell in the room grid
    for (int r = 0; r < ROOM_H; r++) {
        for (int c = 0; c < ROOM_W; c++) {
            int sr = ROOM_OFF_ROW + r;
            int sc = ROOM_OFF_COL + c;

            // Determine if wall position
            bool isTopWall    = (r == 0);
            bool isBottomWall = (r == ROOM_H - 1);
            bool isLeftWall   = (c == 0);
            bool isRightWall  = (c == ROOM_W - 1);
            bool isWall = isTopWall || isBottomWall || isLeftWall || isRightWall;

            if (!isWall) {
                // Floor cell
                attron(COLOR_PAIR(CP_DEFAULT) | A_DIM);
                mvaddch(sr, sc, '.');
                attroff(COLOR_PAIR(CP_DEFAULT) | A_DIM);
                continue;
            }

            // --- Wall or door? ---
            char ch = '#';
            int  cp = CP_WALL;
            int  attr = 0;

            // North door
            if (isTopWall && c == DOOR_N_COL && room.north != -1) {
                ch   = room.northLock ? 'D' : '_';
                cp   = room.northLock ? CP_DOOR_CLO : CP_DOOR_OPN;
                attr = A_BOLD;
            }
            // South door
            else if (isBottomWall && c == DOOR_S_COL && room.south != -1) {
                ch   = room.southLock ? 'D' : '_';
                cp   = room.southLock ? CP_DOOR_CLO : CP_DOOR_OPN;
                attr = A_BOLD;
            }
            // East door
            else if (isRightWall && r == DOOR_E_ROW && room.east != -1) {
                ch   = room.eastLock ? 'D' : '_';
                cp   = room.eastLock ? CP_DOOR_CLO : CP_DOOR_OPN;
                attr = A_BOLD;
            }
            // West door
            else if (isLeftWall && r == DOOR_W_ROW && room.west != -1) {
                ch   = room.westLock ? 'D' : '_';
                cp   = room.westLock ? CP_DOOR_CLO : CP_DOOR_OPN;
                attr = A_BOLD;
            }

            attron(COLOR_PAIR(cp) | attr);
            mvaddch(sr, sc, ch);
            attroff(COLOR_PAIR(cp) | attr);
        }
    }
}

// ============================================================
// Entity drawing
// ============================================================
static void drawEntities(const GameState& gs) {
    const Room& room = gs.rooms[gs.curRoom];

    // Draw items
    for (int i = 0; i < room.numItems; i++) {
        const Item& item = room.items[i];
        if (item.collected) continue;
        int sr = ROOM_OFF_ROW + item.pos.r;
        int sc = ROOM_OFF_COL + item.pos.c;
        attron(COLOR_PAIR(itemColor(item.type)) | A_BOLD);
        mvaddch(sr, sc, itemGlyph(item.type));
        attroff(COLOR_PAIR(itemColor(item.type)) | A_BOLD);
    }

    // Draw enemies
    for (int i = 0; i < room.numEnemies; i++) {
        const Enemy& e = room.enemies[i];
        if (!e.alive) continue;
        int sr = ROOM_OFF_ROW + e.pos.r;
        int sc = ROOM_OFF_COL + e.pos.c;

        bool isBoss = (e.type == ENT_BOSS);
        attron(COLOR_PAIR(enemyColor(e.type)) | A_BOLD | (isBoss ? A_BLINK : 0));
        mvaddch(sr, sc, enemyGlyph(e.type));
        attroff(COLOR_PAIR(enemyColor(e.type)) | A_BOLD | (isBoss ? A_BLINK : 0));

        // HP bar for boss
        if (isBoss && e.maxHp > 0) {
            int barLen = 10;
            int filled = (e.hp * barLen) / e.maxHp;
            int brow = sr + 1;
            int bcol = sc - barLen/2;
            if (brow < TERM_ROWS && bcol >= 0 && bcol + barLen < TERM_COLS) {
                attron(COLOR_PAIR(CP_OVER) | A_BOLD);
                for (int b = 0; b < barLen; b++) {
                    mvaddch(brow, bcol + b, b < filled ? '#' : '-');
                }
                attroff(COLOR_PAIR(CP_OVER) | A_BOLD);
            }
        }
    }

    // Draw player (on top of everything)
    const Player& p = gs.player;
    int psr = ROOM_OFF_ROW + p.pos.r;
    int psc = ROOM_OFF_COL + p.pos.c;
    attron(COLOR_PAIR(CP_PLAYER) | A_BOLD | A_REVERSE);
    mvaddch(psr, psc, playerGlyph(p.facing));
    attroff(COLOR_PAIR(CP_PLAYER) | A_BOLD | A_REVERSE);
}

// ============================================================
// Status bar
// ============================================================
static void drawStatus(const GameState& gs) {
    int row = ROOM_OFF_ROW + ROOM_H;  // row 21

    // Clear line
    move(row, 0);
    for (int c = 0; c < TERM_COLS; c++) addch(' ');

    if (gs.msgTtl > 0) {
        int cp = (strstr(gs.msg, "Golpe") || strstr(gs.msg, "Necesitas")) ? CP_OVER : CP_STAT;
        attron(COLOR_PAIR(cp) | A_BOLD);
        mvprintw(row, 2, "%s", gs.msg);
        attroff(COLOR_PAIR(cp) | A_BOLD);
    }

    // Minimap hint on row 22
    attron(COLOR_PAIR(CP_DEFAULT) | A_DIM);
    mvprintw(row + 1, 2, "Habitaciones visitadas: ");
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (gs.rooms[i].visited) {
            attron(COLOR_PAIR(CP_WIN));
            mvprintw(row + 1, 26 + i*4, "[%d]", i + 1);
            attroff(COLOR_PAIR(CP_WIN));
        } else {
            attron(COLOR_PAIR(CP_DEFAULT) | A_DIM);
            mvprintw(row + 1, 26 + i*4, "[?]");
        }
    }
    attroff(COLOR_PAIR(CP_DEFAULT) | A_DIM);
    attron(COLOR_PAIR(CP_STAT) | A_DIM);
    mvprintw(row + 1, 50, "P:Pausa  Z:Atacar  Q:Menu");
    attroff(COLOR_PAIR(CP_STAT) | A_DIM);
}

// ============================================================
// Full game screen
// ============================================================
void drawGame(const GameState& gs) {
    bkgd(COLOR_PAIR(CP_DEFAULT));
    drawHUD(gs);
    drawRoom(gs);
    drawEntities(gs);
    drawStatus(gs);
}

// ============================================================
// Pause overlay
// ============================================================
void drawPause() {
    int boxH = 7, boxW = 34;
    int boxR = (TERM_ROWS - boxH) / 2;
    int boxC = (TERM_COLS - boxW) / 2;

    // Shadow
    attron(COLOR_PAIR(CP_DEFAULT));
    for (int r = boxR; r < boxR + boxH; r++)
        for (int c = boxC; c < boxC + boxW; c++)
            mvaddch(r, c, ' ');
    attroff(COLOR_PAIR(CP_DEFAULT));

    drawBox(boxR, boxC, boxH, boxW, COLOR_PAIR(CP_WIN) | A_BOLD);

    attron(COLOR_PAIR(CP_WIN) | A_BOLD);
    mvprintw(boxR + 1, boxC + (boxW - 8) / 2,  " PAUSA ");
    attroff(COLOR_PAIR(CP_WIN) | A_BOLD);

    attron(COLOR_PAIR(CP_NORM));
    mvprintw(boxR + 3, boxC + 3, "P / Enter : Reanudar");
    mvprintw(boxR + 5, boxC + 3, "Q / ESC   : Menu");
    attroff(COLOR_PAIR(CP_NORM));
}

// ============================================================
// Game Over screen
// ============================================================
void drawGameOver(const GameState& gs) {
    bkgd(COLOR_PAIR(CP_DEFAULT));

    attron(COLOR_PAIR(CP_OVER) | A_BOLD);
    printCenter(5,  "  ___    _   __  __  ___    ___  _   _ ___  ____  ", 0);
    printCenter(6,  " / __|  /_\\  |  \\/  || __|  / _ \\| | | | __||  _ \\ ", 0);
    printCenter(7,  "| (_ | / _ \\ | |\\/| || _|  | (_) | |_| | _| | |/ / ", 0);
    printCenter(8,  " \\___|/_/ \\_\\|_|  |_||___|  \\___/ \\___/|___||_|_\\  ", 0);
    attroff(COLOR_PAIR(CP_OVER) | A_BOLD);

    attron(COLOR_PAIR(CP_TTL) | A_BOLD);
    char scoreStr[64];
    snprintf(scoreStr, 63, "Puntuacion final: %d", gs.player.score);
    printCenter(11, scoreStr, 0);
    attroff(COLOR_PAIR(CP_TTL) | A_BOLD);

    attron(COLOR_PAIR(CP_NORM));
    printCenter(13, "Link ha caido en combate...", 0);

    int visited = 0;
    for (int i = 0; i < MAX_ROOMS; i++) if (gs.rooms[i].visited) visited++;
    char visitStr[64];
    snprintf(visitStr, 63, "Habitaciones visitadas: %d / %d", visited, MAX_ROOMS);
    printCenter(15, visitStr, 0);
    attroff(COLOR_PAIR(CP_NORM));

    attron(COLOR_PAIR(CP_SEL) | A_BOLD);
    printCenter(19, "  Enter: Guardar puntuacion  ", 0);
    attroff(COLOR_PAIR(CP_SEL) | A_BOLD);

    attron(COLOR_PAIR(CP_STAT));
    printCenter(21, "R: Reiniciar  |  Q: Menu principal", 0);
    attroff(COLOR_PAIR(CP_STAT));
}

// ============================================================
// Victory screen
// ============================================================
void drawVictory(const GameState& gs) {
    bkgd(COLOR_PAIR(CP_DEFAULT));

    attron(COLOR_PAIR(CP_WIN) | A_BOLD);
    printCenter(3,  " __   __ _  ___  ____  ___  ____  _  _    __    ", 0);
    printCenter(4,  " \\ \\ / /| ||   \\|_  _|/ _ \\| __ )| || |  /_\\   ", 0);
    printCenter(5,  "  \\ V / | || |) | | || (_) |    / | \\| | / _ \\  ", 0);
    printCenter(6,  "   \\_/  |_||___/  |_| \\___/|_|_\\ |_||_|/_/ \\_\\ ", 0);
    attroff(COLOR_PAIR(CP_WIN) | A_BOLD);

    // Triforce
    attron(COLOR_PAIR(CP_COIN) | A_BOLD);
    printCenter(9,  "    /\\      /\\", 0);
    printCenter(10, "   /  \\    /  \\", 0);
    printCenter(11, "  /    \\  /    \\", 0);
    printCenter(12, " /------\\/------\\", 0);
    printCenter(13, "     /\\", 0);
    printCenter(14, "    /  \\", 0);
    printCenter(15, "   /    \\", 0);
    printCenter(16, "  /------\\", 0);
    attroff(COLOR_PAIR(CP_COIN) | A_BOLD);

    attron(COLOR_PAIR(CP_TTL) | A_BOLD);
    char scoreStrV[64];
    snprintf(scoreStrV, 63, "Puntuacion Final: %d puntos", gs.player.score);
    printCenter(19, scoreStrV, 0);
    attroff(COLOR_PAIR(CP_TTL) | A_BOLD);

    attron(COLOR_PAIR(CP_SEL) | A_BOLD);
    printCenter(21, "  Enter: Guardar puntuacion  ", 0);
    attroff(COLOR_PAIR(CP_SEL) | A_BOLD);

    attron(COLOR_PAIR(CP_STAT));
    printCenter(23, "R: Reiniciar  |  Q: Menu principal", 0);
    attroff(COLOR_PAIR(CP_STAT));
}

// ============================================================
// Enter name screen
// ============================================================
void drawEnterName(const char* nameBuf, int nameLen) {
    bkgd(COLOR_PAIR(CP_DEFAULT));

    attron(COLOR_PAIR(CP_TTL) | A_BOLD);
    printCenter(6, "INGRESA TU NOMBRE", 0);
    attroff(COLOR_PAIR(CP_TTL) | A_BOLD);

    attron(COLOR_PAIR(CP_NORM));
    printCenter(8, "para guardar tu puntaje en el registro de honor:", 0);
    attroff(COLOR_PAIR(CP_NORM));

    // Input box
    int boxR = 10, boxC = (TERM_COLS - NAME_LEN - 6) / 2, boxW = NAME_LEN + 6, boxH = 3;
    drawBox(boxR, boxC, boxH, boxW, COLOR_PAIR(CP_WIN) | A_BOLD);

    // Name with cursor
    attron(COLOR_PAIR(CP_WIN) | A_BOLD);
    mvprintw(boxR + 1, boxC + 2, "%-12s", nameBuf);
    attroff(COLOR_PAIR(CP_WIN) | A_BOLD);

    // Blinking cursor
    if ((nameLen < NAME_LEN)) {
        attron(COLOR_PAIR(CP_SEL) | A_BOLD);
        mvaddch(boxR + 1, boxC + 2 + nameLen, ' ');
        attroff(COLOR_PAIR(CP_SEL) | A_BOLD);
    }

    attron(COLOR_PAIR(CP_STAT));
    printCenter(14, "Enter: Confirmar  |  Backspace: Borrar  |  ESC: Cancelar", 0);
    attroff(COLOR_PAIR(CP_STAT));

    curs_set(0); // keep cursor hidden
}

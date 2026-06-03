#pragma once
#include "game.h"
#include "scores.h"

void rendererInit();
void rendererCleanup();

void drawMainMenu(int selected);
void drawModeSelect(int selected);
void drawInstructions(int page);
void drawScores(const ScoreEntry entries[], int count);
void drawGame(const GameState& gs);
void drawPause();
void drawGameOver(const GameState& gs);
void drawVictory(const GameState& gs);
void drawEnterName(const char* nameBuf, int nameLen);

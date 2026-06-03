#pragma once
#include "game.h"

#define SCORES_FILE "data/scores.txt"

void loadScores(ScoreEntry entries[], int& count);
void saveScores(const ScoreEntry entries[], int count);
void addScore(ScoreEntry entries[], int& count,
              const char* name, int score, int mode);

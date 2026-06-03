#include "scores.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

void loadScores(ScoreEntry entries[], int& count) {
    count = 0;
    FILE* f = fopen(SCORES_FILE, "r");
    if (!f) return;

    while (count < MAX_SCORES) {
        ScoreEntry e;
        if (fscanf(f, "%12s %d %d", e.name, &e.score, &e.mode) != 3) break;
        entries[count++] = e;
    }
    fclose(f);
}

void saveScores(const ScoreEntry entries[], int count) {
    FILE* f = fopen(SCORES_FILE, "w");
    if (!f) return;

    for (int i = 0; i < count; i++) {
        fprintf(f, "%s %d %d\n", entries[i].name, entries[i].score, entries[i].mode);
    }
    fclose(f);
}

// Insert new score keeping list sorted descending, max MAX_SCORES entries
void addScore(ScoreEntry entries[], int& count,
              const char* name, int score, int mode) {
    ScoreEntry e;
    strncpy(e.name, name, NAME_LEN);
    e.name[NAME_LEN] = '\0';
    e.score = score;
    e.mode  = mode;

    // Find insertion position
    int pos = count;
    for (int i = 0; i < count; i++) {
        if (score > entries[i].score) { pos = i; break; }
    }

    // Shift down
    int newCount = (count < MAX_SCORES) ? count + 1 : MAX_SCORES;
    for (int i = newCount - 1; i > pos; i--) {
        entries[i] = entries[i - 1];
    }
    if (pos < MAX_SCORES) {
        entries[pos] = e;
    }
    count = newCount;
}

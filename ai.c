#include "ai.h"

#include <stdlib.h>

Move choose_ai_move(const ChessBoard *state) {
    Move legal[MAX_MOVES];
    int count = 0;
    int i;
    int best_score = -100000;
    int best_indices[MAX_MOVES];
    int best_count = 0;
    Move fallback = {0, 0, 0, 0, EMPTY};

    generate_legal_moves(state, legal, &count);
    if (count == 0) return fallback;

    for (i = 0; i < count; i++) {
        Piece captured = state->board[legal[i].to_row][legal[i].to_col];
        int score = piece_value(captured);

        /* Give a small bonus for promotion. */
        if (legal[i].promotion != EMPTY) score += 8;

        if (score > best_score) {
            best_score = score;
            best_indices[0] = i;
            best_count = 1;
        } else if (score == best_score && best_count < MAX_MOVES) {
            best_indices[best_count++] = i;
        }
    }

    if (best_count == 0) return legal[rand() % count];
    return legal[best_indices[rand() % best_count]];
}

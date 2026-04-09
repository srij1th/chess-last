#include "board.h"

#include <stdio.h>
#include <string.h>

static int same_color(Piece a, Piece b) {
    return (is_white_piece(a) && is_white_piece(b)) || (is_black_piece(a) && is_black_piece(b));
}

int is_white_piece(Piece p) {
    return p >= WP && p <= WK;
}

int is_black_piece(Piece p) {
    return p >= BP && p <= BK;
}

int is_inside_board(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

char piece_to_char(Piece p) {
    switch (p) {
        case WP: return 'P';
        case WN: return 'N';
        case WB: return 'B';
        case WR: return 'R';
        case WQ: return 'Q';
        case WK: return 'K';
        case BP: return 'p';
        case BN: return 'n';
        case BB: return 'b';
        case BR: return 'r';
        case BQ: return 'q';
        case BK: return 'k';
        default: return '.';
    }
}

int piece_value(Piece p) {
    switch (p) {
        case WP:
        case BP: return 1;
        case WN:
        case BN:
        case WB:
        case BB: return 3;
        case WR:
        case BR: return 5;
        case WQ:
        case BQ: return 9;
        default: return 0;
    }
}

void init_board(ChessBoard *state) {
    int r, c;
    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            state->board[r][c] = EMPTY;
        }
    }

    state->board[0][0] = BR; state->board[0][1] = BN; state->board[0][2] = BB; state->board[0][3] = BQ;
    state->board[0][4] = BK; state->board[0][5] = BB; state->board[0][6] = BN; state->board[0][7] = BR;
    for (c = 0; c < BOARD_SIZE; c++) state->board[1][c] = BP;

    state->board[7][0] = WR; state->board[7][1] = WN; state->board[7][2] = WB; state->board[7][3] = WQ;
    state->board[7][4] = WK; state->board[7][5] = WB; state->board[7][6] = WN; state->board[7][7] = WR;
    for (c = 0; c < BOARD_SIZE; c++) state->board[6][c] = WP;

    state->side_to_move = 0;
    state->white_can_castle_kingside = 1;
    state->white_can_castle_queenside = 1;
    state->black_can_castle_kingside = 1;
    state->black_can_castle_queenside = 1;
    state->en_passant_row = -1;
    state->en_passant_col = -1;
}

void print_board(const ChessBoard *state, int display_style) {
    int r, c;
    (void)display_style;
    printf("\n    a b c d e f g h\n");
    printf("   -----------------\n");
    for (r = 0; r < BOARD_SIZE; r++) {
        printf(" %d |", 8 - r);
        for (c = 0; c < BOARD_SIZE; c++) {
            printf("%c ", piece_to_char(state->board[r][c]));
        }
        printf("| %d\n", 8 - r);
    }
    printf("   -----------------\n");
    printf("    a b c d e f g h\n\n");
}

static void add_move(Move *moves, int *count, int from_r, int from_c, int to_r, int to_c, Piece promotion) {
    if (*count >= MAX_MOVES) return;
    moves[*count].from_row = from_r;
    moves[*count].from_col = from_c;
    moves[*count].to_row = to_r;
    moves[*count].to_col = to_c;
    moves[*count].promotion = promotion;
    (*count)++;
}

static int is_square_attacked(const ChessBoard *state, int row, int col, int by_color) {
    int dr[8] = {-2, -2, -1, -1, 1, 1, 2, 2};
    int dc[8] = {-1, 1, -2, 2, -2, 2, -1, 1};
    int i, r, c;

    int pawn_dir = (by_color == 0) ? -1 : 1;
    int pawn_row = row - pawn_dir;
    if (is_inside_board(pawn_row, col - 1)) {
        Piece p = state->board[pawn_row][col - 1];
        if (by_color == 0 && p == WP) return 1;
        if (by_color == 1 && p == BP) return 1;
    }
    if (is_inside_board(pawn_row, col + 1)) {
        Piece p = state->board[pawn_row][col + 1];
        if (by_color == 0 && p == WP) return 1;
        if (by_color == 1 && p == BP) return 1;
    }

    for (i = 0; i < 8; i++) {
        r = row + dr[i];
        c = col + dc[i];
        if (is_inside_board(r, c)) {
            Piece p = state->board[r][c];
            if (by_color == 0 && p == WN) return 1;
            if (by_color == 1 && p == BN) return 1;
        }
    }

    {
        int dirs[4][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};
        int d;
        for (d = 0; d < 4; d++) {
            r = row + dirs[d][0];
            c = col + dirs[d][1];
            while (is_inside_board(r, c)) {
                Piece p = state->board[r][c];
                if (p != EMPTY) {
                    if (by_color == 0 && (p == WB || p == WQ)) return 1;
                    if (by_color == 1 && (p == BB || p == BQ)) return 1;
                    break;
                }
                r += dirs[d][0];
                c += dirs[d][1];
            }
        }
    }

    {
        int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
        int d;
        for (d = 0; d < 4; d++) {
            r = row + dirs[d][0];
            c = col + dirs[d][1];
            while (is_inside_board(r, c)) {
                Piece p = state->board[r][c];
                if (p != EMPTY) {
                    if (by_color == 0 && (p == WR || p == WQ)) return 1;
                    if (by_color == 1 && (p == BR || p == BQ)) return 1;
                    break;
                }
                r += dirs[d][0];
                c += dirs[d][1];
            }
        }
    }

    for (r = row - 1; r <= row + 1; r++) {
        for (c = col - 1; c <= col + 1; c++) {
            if (r == row && c == col) continue;
            if (!is_inside_board(r, c)) continue;
            if (by_color == 0 && state->board[r][c] == WK) return 1;
            if (by_color == 1 && state->board[r][c] == BK) return 1;
        }
    }

    return 0;
}

int is_in_check(const ChessBoard *state, int color) {
    int r, c;
    Piece king = (color == 0) ? WK : BK;
    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            if (state->board[r][c] == king) {
                return is_square_attacked(state, r, c, 1 - color);
            }
        }
    }
    return 0;
}

static void generate_pseudo_moves(const ChessBoard *state, Move *moves, int *count) {
    int r, c;
    *count = 0;

    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            Piece p = state->board[r][c];
            int color = state->side_to_move;
            if (p == EMPTY) continue;
            if (color == 0 && !is_white_piece(p)) continue;
            if (color == 1 && !is_black_piece(p)) continue;

            if (p == WP || p == BP) {
                int dir = (p == WP) ? -1 : 1;
                int start_row = (p == WP) ? 6 : 1;
                int promotion_row = (p == WP) ? 0 : 7;
                int nr = r + dir;

                if (is_inside_board(nr, c) && state->board[nr][c] == EMPTY) {
                    if (nr == promotion_row) {
                        add_move(moves, count, r, c, nr, c, (p == WP) ? WQ : BQ);
                    } else {
                        add_move(moves, count, r, c, nr, c, EMPTY);
                    }
                    if (r == start_row && state->board[r + 2 * dir][c] == EMPTY) {
                        add_move(moves, count, r, c, r + 2 * dir, c, EMPTY);
                    }
                }

                {
                    int cols[2] = {c - 1, c + 1};
                    int i;
                    for (i = 0; i < 2; i++) {
                        int nc = cols[i];
                        if (!is_inside_board(nr, nc)) continue;
                        if (state->board[nr][nc] != EMPTY && !same_color(p, state->board[nr][nc])) {
                            if (nr == promotion_row) {
                                add_move(moves, count, r, c, nr, nc, (p == WP) ? WQ : BQ);
                            } else {
                                add_move(moves, count, r, c, nr, nc, EMPTY);
                            }
                        }
                        if (nr == state->en_passant_row && nc == state->en_passant_col) {
                            add_move(moves, count, r, c, nr, nc, EMPTY);
                        }
                    }
                }
            } else if (p == WN || p == BN) {
                int dr[8] = {-2, -2, -1, -1, 1, 1, 2, 2};
                int dc[8] = {-1, 1, -2, 2, -2, 2, -1, 1};
                int i;
                for (i = 0; i < 8; i++) {
                    int nr = r + dr[i], nc = c + dc[i];
                    if (!is_inside_board(nr, nc)) continue;
                    if (state->board[nr][nc] == EMPTY || !same_color(p, state->board[nr][nc])) {
                        add_move(moves, count, r, c, nr, nc, EMPTY);
                    }
                }
            } else if (p == WB || p == BB || p == WR || p == BR || p == WQ || p == BQ) {
                int dirs[8][2] = {
                    {-1,-1},{-1,1},{1,-1},{1,1},
                    {-1,0},{1,0},{0,-1},{0,1}
                };
                int start = (p == WB || p == BB) ? 0 : (p == WR || p == BR) ? 4 : 0;
                int end = (p == WB || p == BB) ? 4 : (p == WR || p == BR) ? 8 : 8;
                int d;
                for (d = start; d < end; d++) {
                    int nr = r + dirs[d][0], nc = c + dirs[d][1];
                    while (is_inside_board(nr, nc)) {
                        if (state->board[nr][nc] == EMPTY) {
                            add_move(moves, count, r, c, nr, nc, EMPTY);
                        } else {
                            if (!same_color(p, state->board[nr][nc])) {
                                add_move(moves, count, r, c, nr, nc, EMPTY);
                            }
                            break;
                        }
                        nr += dirs[d][0];
                        nc += dirs[d][1];
                    }
                }
            } else if (p == WK || p == BK) {
                int nr, nc;
                for (nr = r - 1; nr <= r + 1; nr++) {
                    for (nc = c - 1; nc <= c + 1; nc++) {
                        if (nr == r && nc == c) continue;
                        if (!is_inside_board(nr, nc)) continue;
                        if (state->board[nr][nc] == EMPTY || !same_color(p, state->board[nr][nc])) {
                            add_move(moves, count, r, c, nr, nc, EMPTY);
                        }
                    }
                }

                if (p == WK && r == 7 && c == 4) {
                    if (state->white_can_castle_kingside &&
                        state->board[7][5] == EMPTY && state->board[7][6] == EMPTY &&
                        !is_square_attacked(state, 7, 4, 1) &&
                        !is_square_attacked(state, 7, 5, 1) &&
                        !is_square_attacked(state, 7, 6, 1)) {
                        add_move(moves, count, 7, 4, 7, 6, EMPTY);
                    }
                    if (state->white_can_castle_queenside &&
                        state->board[7][1] == EMPTY && state->board[7][2] == EMPTY && state->board[7][3] == EMPTY &&
                        !is_square_attacked(state, 7, 4, 1) &&
                        !is_square_attacked(state, 7, 3, 1) &&
                        !is_square_attacked(state, 7, 2, 1)) {
                        add_move(moves, count, 7, 4, 7, 2, EMPTY);
                    }
                }
                if (p == BK && r == 0 && c == 4) {
                    if (state->black_can_castle_kingside &&
                        state->board[0][5] == EMPTY && state->board[0][6] == EMPTY &&
                        !is_square_attacked(state, 0, 4, 0) &&
                        !is_square_attacked(state, 0, 5, 0) &&
                        !is_square_attacked(state, 0, 6, 0)) {
                        add_move(moves, count, 0, 4, 0, 6, EMPTY);
                    }
                    if (state->black_can_castle_queenside &&
                        state->board[0][1] == EMPTY && state->board[0][2] == EMPTY && state->board[0][3] == EMPTY &&
                        !is_square_attacked(state, 0, 4, 0) &&
                        !is_square_attacked(state, 0, 3, 0) &&
                        !is_square_attacked(state, 0, 2, 0)) {
                        add_move(moves, count, 0, 4, 0, 2, EMPTY);
                    }
                }
            }
        }
    }
}

void apply_move(ChessBoard *state, const Move *move) {
    Piece moving = state->board[move->from_row][move->from_col];
    Piece target = state->board[move->to_row][move->to_col];
    int old_ep_row = state->en_passant_row;
    int old_ep_col = state->en_passant_col;

    state->en_passant_row = -1;
    state->en_passant_col = -1;

    if (moving == WK) {
        state->white_can_castle_kingside = 0;
        state->white_can_castle_queenside = 0;
    } else if (moving == BK) {
        state->black_can_castle_kingside = 0;
        state->black_can_castle_queenside = 0;
    } else if (moving == WR && move->from_row == 7 && move->from_col == 0) {
        state->white_can_castle_queenside = 0;
    } else if (moving == WR && move->from_row == 7 && move->from_col == 7) {
        state->white_can_castle_kingside = 0;
    } else if (moving == BR && move->from_row == 0 && move->from_col == 0) {
        state->black_can_castle_queenside = 0;
    } else if (moving == BR && move->from_row == 0 && move->from_col == 7) {
        state->black_can_castle_kingside = 0;
    }

    if (target == WR && move->to_row == 7 && move->to_col == 0) state->white_can_castle_queenside = 0;
    if (target == WR && move->to_row == 7 && move->to_col == 7) state->white_can_castle_kingside = 0;
    if (target == BR && move->to_row == 0 && move->to_col == 0) state->black_can_castle_queenside = 0;
    if (target == BR && move->to_row == 0 && move->to_col == 7) state->black_can_castle_kingside = 0;

    if ((moving == WP || moving == BP) && move->to_col != move->from_col && target == EMPTY) {
        int cap_row = (moving == WP) ? move->to_row + 1 : move->to_row - 1;
        state->board[cap_row][move->to_col] = EMPTY;
    }

    state->board[move->to_row][move->to_col] = moving;
    state->board[move->from_row][move->from_col] = EMPTY;

    if (moving == WK && move->from_row == 7 && move->from_col == 4 && move->to_row == 7 && move->to_col == 6) {
        state->board[7][5] = WR;
        state->board[7][7] = EMPTY;
    } else if (moving == WK && move->from_row == 7 && move->from_col == 4 && move->to_row == 7 && move->to_col == 2) {
        state->board[7][3] = WR;
        state->board[7][0] = EMPTY;
    } else if (moving == BK && move->from_row == 0 && move->from_col == 4 && move->to_row == 0 && move->to_col == 6) {
        state->board[0][5] = BR;
        state->board[0][7] = EMPTY;
    } else if (moving == BK && move->from_row == 0 && move->from_col == 4 && move->to_row == 0 && move->to_col == 2) {
        state->board[0][3] = BR;
        state->board[0][0] = EMPTY;
    }

    if (moving == WP && move->to_row == 0) {
        state->board[move->to_row][move->to_col] = (move->promotion == EMPTY) ? WQ : move->promotion;
    } else if (moving == BP && move->to_row == 7) {
        state->board[move->to_row][move->to_col] = (move->promotion == EMPTY) ? BQ : move->promotion;
    }

    if (moving == WP && move->from_row == 6 && move->to_row == 4) {
        state->en_passant_row = 5;
        state->en_passant_col = move->from_col;
    } else if (moving == BP && move->from_row == 1 && move->to_row == 3) {
        state->en_passant_row = 2;
        state->en_passant_col = move->from_col;
    }

    if ((moving == WP || moving == BP) && move->to_row == old_ep_row && move->to_col == old_ep_col &&
        move->from_col != move->to_col && target == EMPTY) {
        /* En passant capture already removed above. */
    }

    state->side_to_move = 1 - state->side_to_move;
}

void generate_legal_moves(const ChessBoard *state, Move *moves, int *move_count) {
    Move pseudo[MAX_MOVES];
    int pseudo_count = 0;
    int i;
    *move_count = 0;

    generate_pseudo_moves(state, pseudo, &pseudo_count);

    for (i = 0; i < pseudo_count; i++) {
        ChessBoard copy = *state;
        int color = state->side_to_move;
        apply_move(&copy, &pseudo[i]);
        if (!is_in_check(&copy, color)) {
            if (*move_count < MAX_MOVES) {
                moves[*move_count] = pseudo[i];
                (*move_count)++;
            }
        }
    }
}

int is_move_legal(const ChessBoard *state, const Move *move) {
    Move legal[MAX_MOVES];
    int count, i;
    generate_legal_moves(state, legal, &count);
    for (i = 0; i < count; i++) {
        if (legal[i].from_row == move->from_row &&
            legal[i].from_col == move->from_col &&
            legal[i].to_row == move->to_row &&
            legal[i].to_col == move->to_col) {
            return 1;
        }
    }
    return 0;
}

int is_checkmate(const ChessBoard *state, int color) {
    Move legal[MAX_MOVES];
    int count;
    ChessBoard copy = *state;
    copy.side_to_move = color;
    generate_legal_moves(&copy, legal, &count);
    return is_in_check(state, color) && count == 0;
}

int is_stalemate(const ChessBoard *state, int color) {
    Move legal[MAX_MOVES];
    int count;
    ChessBoard copy = *state;
    copy.side_to_move = color;
    generate_legal_moves(&copy, legal, &count);
    return !is_in_check(state, color) && count == 0;
}

void move_to_text(const Move *move, char *out, int out_size) {
    char f1 = (char)('a' + move->from_col);
    char r1 = (char)('8' - move->from_row);
    char f2 = (char)('a' + move->to_col);
    char r2 = (char)('8' - move->to_row);
    if (out_size <= 0) return;
    snprintf(out, (size_t)out_size, "%c%c %c%c", f1, r1, f2, r2);
}

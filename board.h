#ifndef BOARD_H
#define BOARD_H

#define BOARD_SIZE 8
#define MAX_MOVES 256

typedef enum {
    EMPTY = 0,
    WP, WN, WB, WR, WQ, WK,
    BP, BN, BB, BR, BQ, BK
} Piece;

typedef struct {
    int from_row;
    int from_col;
    int to_row;
    int to_col;
    Piece promotion;
} Move;

typedef struct {
    Piece board[BOARD_SIZE][BOARD_SIZE];
    int side_to_move; /* 0 = white, 1 = black */
    int white_can_castle_kingside;
    int white_can_castle_queenside;
    int black_can_castle_kingside;
    int black_can_castle_queenside;
    int en_passant_row;
    int en_passant_col;
} ChessBoard;

void init_board(ChessBoard *state);
void print_board(const ChessBoard *state, int display_style);

int is_white_piece(Piece p);
int is_black_piece(Piece p);
int is_inside_board(int row, int col);
char piece_to_char(Piece p);

void generate_legal_moves(const ChessBoard *state, Move *moves, int *move_count);
int is_move_legal(const ChessBoard *state, const Move *move);
void apply_move(ChessBoard *state, const Move *move);

int is_in_check(const ChessBoard *state, int color);
int is_checkmate(const ChessBoard *state, int color);
int is_stalemate(const ChessBoard *state, int color);

int piece_value(Piece p);
void move_to_text(const Move *move, char *out, int out_size);

#endif

#include "game.h"

#include "ai.h"
#include "board.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_algebraic_move(const char *input, Move *move) {
    char from[3], to[3];
    if (sscanf(input, "%2s %2s", from, to) != 2) return 0;
    if (strlen(from) != 2 || strlen(to) != 2) return 0;
    if (from[0] < 'a' || from[0] > 'h' || to[0] < 'a' || to[0] > 'h') return 0;
    if (from[1] < '1' || from[1] > '8' || to[1] < '1' || to[1] > '8') return 0;

    move->from_col = from[0] - 'a';
    move->from_row = 8 - (from[1] - '0');
    move->to_col = to[0] - 'a';
    move->to_row = 8 - (to[1] - '0');
    move->promotion = EMPTY;
    return 1;
}

static int parse_numeric_move(const char *input, Move *move) {
    int r1, c1, r2, c2;
    if (sscanf(input, "%d %d %d %d", &r1, &c1, &r2, &c2) != 4) return 0;
    if (r1 < 1 || r1 > 8 || r2 < 1 || r2 > 8 || c1 < 1 || c1 > 8 || c2 < 1 || c2 > 8) return 0;

    move->from_row = 8 - r1;
    move->from_col = c1 - 1;
    move->to_row = 8 - r2;
    move->to_col = c2 - 1;
    move->promotion = EMPTY;
    return 1;
}

void show_instructions(void) {
    printf("\n========== CHESS INSTRUCTIONS ==========\n");
    printf("1) Objective: Checkmate the opponent king.\n");
    printf("2) White moves first, then turns alternate.\n");
    printf("3) Special rules supported:\n");
    printf("   - Castling\n");
    printf("   - En passant\n");
    printf("   - Pawn promotion (auto-promotes to Queen)\n");
    printf("4) Win conditions:\n");
    printf("   - Checkmate: opponent king is in check with no legal moves.\n");
    printf("   - Draw by stalemate: no legal moves and king is not in check.\n");
    printf("5) Controls:\n");
    printf("   - Algebraic style: e2 e4\n");
    printf("   - Numeric style  : 2 5 4 5  (row col row col)\n");
    printf("     Row 1 is White home row (rank 1), row 8 is Black home row.\n");
    printf("========================================\n\n");
}

void settings_menu(UserProfile *user) {
    char input[INPUT_SIZE];
    int choice = 0;
    while (1) {
        printf("\n--- Settings ---\n");
        printf("1) Keybinding style (current: %s)\n", user->keybinding_style == 0 ? "Algebraic" : "Numeric");
        printf("2) Board display style (current: %d)\n", user->board_style);
        printf("3) Back\n");
        printf("Choose option: ");
        read_line(input, sizeof(input));
        choice = atoi(input);

        if (choice == 1) {
            printf("Select keybinding style:\n");
            printf("1) Algebraic (e2 e4)\n");
            printf("2) Numeric   (2 5 4 5)\n");
            printf("Choice: ");
            read_line(input, sizeof(input));
            if (atoi(input) == 1) user->keybinding_style = 0;
            else if (atoi(input) == 2) user->keybinding_style = 1;
            printf("Keybinding updated.\n");
        } else if (choice == 2) {
            printf("Select board style:\n");
            printf("0) Classic\n");
            printf("1) Alternate (same symbols, reserved for future style)\n");
            printf("Choice: ");
            read_line(input, sizeof(input));
            if (atoi(input) == 0 || atoi(input) == 1) {
                user->board_style = atoi(input);
                printf("Board style updated.\n");
            } else {
                printf("Invalid style.\n");
            }
        } else if (choice == 3) {
            break;
        } else {
            printf("Invalid option.\n");
        }
    }
}

static int parse_user_move(const char *input, int key_style, Move *move) {
    if (key_style == 0) return parse_algebraic_move(input, move);
    return parse_numeric_move(input, move);
}

void start_new_game(UserProfile *user) {
    ChessBoard board;
    char input[INPUT_SIZE];
    Move move;
    Move ai_move;
    int human_color = 0;
    int game_over = 0;

    init_board(&board);
    printf("\nChoose your side:\n1) White (you move first)\n2) Black (AI moves first)\nChoice: ");
    read_line(input, sizeof(input));
    if (atoi(input) == 2) human_color = 1;

    while (!game_over) {
        Move legal[MAX_MOVES];
        int legal_count = 0;
        int current = board.side_to_move;

        print_board(&board, user->board_style);
        generate_legal_moves(&board, legal, &legal_count);

        if (legal_count == 0) {
            if (is_in_check(&board, current)) {
                if (current == human_color) {
                    printf("Checkmate! AI wins.\n");
                    user->losses++;
                } else {
                    printf("Checkmate! You win.\n");
                    user->wins++;
                }
            } else {
                printf("Stalemate! Draw.\n");
                user->draws++;
            }
            user->games_played++;
            game_over = 1;
            continue;
        }

        if (is_in_check(&board, current)) {
            printf("%s is in check.\n", current == 0 ? "White" : "Black");
        }

        if (current == human_color) {
            printf("Your move (%s): ", user->keybinding_style == 0 ? "example: e2 e4" : "example: 2 5 4 5");
            read_line(input, sizeof(input));

            if (equals_ignore_case(input, "quit")) {
                printf("Game ended early. Not counted in stats.\n");
                return;
            }

            if (!parse_user_move(input, user->keybinding_style, &move)) {
                printf("Invalid input format. Try again.\n");
                continue;
            }
            if (!is_move_legal(&board, &move)) {
                printf("Illegal move. Try again.\n");
                continue;
            }
            apply_move(&board, &move);
        } else {
            ai_move = choose_ai_move(&board);
            if (!is_move_legal(&board, &ai_move)) {
                /* Safety fallback, should rarely happen. */
                ai_move = legal[rand() % legal_count];
            }
            {
                char text[16];
                move_to_text(&ai_move, text, sizeof(text));
                printf("AI move: %s\n", text);
            }
            apply_move(&board, &ai_move);
        }
    }
}

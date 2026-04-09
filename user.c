#include "user.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USER_FILE "users.txt"
#define TEMP_USER_FILE "users.tmp"

static int parse_user_line(const char *line, UserProfile *user) {
    return sscanf(
               line,
               "%31[^|]|%31[^|]|%d|%d|%d|%d|%d|%d",
               user->username,
               user->password,
               &user->games_played,
               &user->wins,
               &user->losses,
               &user->draws,
               &user->keybinding_style,
               &user->board_style
           ) == 8;
}

static void write_user_line(FILE *fp, const UserProfile *user) {
    fprintf(
        fp,
        "%s|%s|%d|%d|%d|%d|%d|%d\n",
        user->username,
        user->password,
        user->games_played,
        user->wins,
        user->losses,
        user->draws,
        user->keybinding_style,
        user->board_style
    );
}

int create_profile(const char *username, const char *password) {
    FILE *fp;
    char line[256];
    UserProfile user;

    if (strlen(username) == 0 || strlen(password) == 0) return 0;
    if (strchr(username, '|') || strchr(password, '|')) return 0;

    fp = fopen(USER_FILE, "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            if (parse_user_line(line, &user) && strcmp(user.username, username) == 0) {
                fclose(fp);
                return 0;
            }
        }
        fclose(fp);
    }

    fp = fopen(USER_FILE, "a");
    if (!fp) return 0;

    memset(&user, 0, sizeof(user));
    strncpy(user.username, username, sizeof(user.username) - 1);
    strncpy(user.password, password, sizeof(user.password) - 1);
    user.keybinding_style = 0;
    user.board_style = 0;
    write_user_line(fp, &user);
    fclose(fp);
    return 1;
}

int login_user(const char *username, const char *password, UserProfile *out_user) {
    FILE *fp = fopen(USER_FILE, "r");
    char line[256];
    UserProfile user;

    if (!fp) return 0;
    while (fgets(line, sizeof(line), fp)) {
        if (parse_user_line(line, &user)) {
            if (strcmp(user.username, username) == 0 && strcmp(user.password, password) == 0) {
                fclose(fp);
                *out_user = user;
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

int save_user_profile(const UserProfile *user) {
    FILE *in = fopen(USER_FILE, "r");
    FILE *out = fopen(TEMP_USER_FILE, "w");
    char line[256];
    UserProfile current;
    int replaced = 0;

    if (!out) {
        if (in) fclose(in);
        return 0;
    }

    if (in) {
        while (fgets(line, sizeof(line), in)) {
            if (parse_user_line(line, &current) && strcmp(current.username, user->username) == 0) {
                write_user_line(out, user);
                replaced = 1;
            } else {
                fputs(line, out);
            }
        }
        fclose(in);
    }

    if (!replaced) {
        write_user_line(out, user);
    }

    fclose(out);

    remove(USER_FILE);
    if (rename(TEMP_USER_FILE, USER_FILE) != 0) {
        return 0;
    }
    return 1;
}

int reset_user_stats(UserProfile *user) {
    if (!user) return 0;
    user->games_played = 0;
    user->wins = 0;
    user->losses = 0;
    user->draws = 0;
    return save_user_profile(user);
}

void print_user_stats(const UserProfile *user) {
    printf("\n--- Player Stats ---\n");
    printf("Username     : %s\n", user->username);
    printf("Games Played : %d\n", user->games_played);
    printf("Wins         : %d\n", user->wins);
    printf("Losses       : %d\n", user->losses);
    printf("Draws        : %d\n", user->draws);
    printf("--------------------\n\n");
}

void print_sample_user_file_format(void) {
    printf("\nSample users.txt format:\n");
    printf("username|password|games_played|wins|losses|draws|keybinding_style|board_style\n");
    printf("student1|pass123|10|5|3|2|0|0\n\n");
}

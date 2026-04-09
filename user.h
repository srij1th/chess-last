#ifndef USER_H
#define USER_H

typedef struct {
    char username[32];
    char password[32];
    int games_played;
    int wins;
    int losses;
    int draws;
    int keybinding_style; /* 0 = algebraic, 1 = numeric */
    int board_style;      /* currently 0/1 */
} UserProfile;

int create_profile(const char *username, const char *password);
int login_user(const char *username, const char *password, UserProfile *out_user);
int save_user_profile(const UserProfile *user);
int reset_user_stats(UserProfile *user);

void print_user_stats(const UserProfile *user);
void print_sample_user_file_format(void);

#endif

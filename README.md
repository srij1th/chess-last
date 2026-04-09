# Full-Stack Chess Game (C + HTML/CSS/JS)

This project keeps all chess logic in **pure C backend** and adds a simple **frontend** using HTML, CSS, and JavaScript.

## Backend files (C)

- `main.c`
- `server.c` / `server.h`
- `board.c` / `board.h`
- `game.c` / `game.h` (CLI helper module kept for reference)
- `ai.c` / `ai.h`
- `user.c` / `user.h`
- `utils.c` / `utils.h`

## Frontend files

- `index.html`
- `dashboard.html`
- `game.html`
- `stats.html`
- `settings.html`
- `instructions.html`
- `styles.css`
- `script.js`

## API endpoints (C server)

- `POST /login`
- `POST /signup`
- `POST /logout`
- `POST /start-game`
- `POST /move`
- `POST /ai-move`
- `GET /get-board`
- `GET /get-stats`
- `POST /settings`
- `POST /reset-stats`

## Compile backend (gcc)

### Windows (MinGW gcc)

```bash
gcc main.c server.c board.c game.c ai.c user.c utils.c -o chess_server.exe -lws2_32
```

### Linux/macOS

```bash
gcc main.c server.c board.c game.c ai.c user.c utils.c -o chess_server
```

## Run server

### Windows

```bash
.\chess_server.exe
```

### Linux/macOS

```bash
./chess_server
```

Server starts at:

- `http://localhost:8080`
- Open in browser: `http://localhost:8080/index.html`

## Frontend flow

1. Login/Signup in `index.html`
2. Open Dashboard
3. Start Game and play by clicking pieces/squares
4. View stats and settings using buttons
5. Game page auto-starts a fresh game when opened/refreshed

## New UX features

- AI delay (about 1.4 seconds) for realistic response
- Last move highlight on board (from and to squares)
- Move log panel with live updates
- Theme toggle (light/dark) applied instantly
- Reset stats button (updates backend `users.txt`)

## Sample user data file format (`users.txt`)

Each line stores one user:

```txt
username|password|games_played|wins|losses|draws|keybinding_style|board_style
student1|pass123|10|5|3|2|0|0
```

- `keybinding_style`: `0` = algebraic, `1` = numeric
- `board_style`: `0` or `1`

## Notes

- Backend is intentionally simple and beginner-friendly.
- JSON parsing is basic (works for this project format).
- Single running session/game state is maintained by server for simplicity.

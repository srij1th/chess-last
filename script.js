const API = "";
const REQUEST_TIMEOUT_MS = 6000;

const PIECE_MAP = {
  P: "♙", N: "♘", B: "♗", R: "♖", Q: "♕", K: "♔",
  p: "♟", n: "♞", b: "♝", r: "♜", q: "♛", k: "♚",
  ".": ""
};

const appState = {
  board: null,
  legalMoves: [],
  selected: null,
  lastMove: null,
  moveHistory: []
};

function setMessage(id, text, ok) {
  const el = document.getElementById(id);
  if (!el) return;
  el.textContent = text || "";
  el.style.color = ok ? "#166534" : "#b91c1c";
}

async function fetchWithTimeout(url, options) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), REQUEST_TIMEOUT_MS);
  try {
    return await fetch(url, { ...options, signal: controller.signal });
  } finally {
    clearTimeout(timer);
  }
}

async function apiPost(path, data) {
  const res = await fetchWithTimeout(API + path, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(data || {})
  });
  return res.json();
}

async function apiGet(path) {
  const res = await fetchWithTimeout(API + path, {});
  return res.json();
}

function go(page) {
  window.location.href = page;
}

function currentPage() {
  return document.body.dataset.page || "";
}

function saveUsername(name) {
  localStorage.setItem("chess_user", name);
}

function getUsername() {
  return localStorage.getItem("chess_user") || "";
}

function saveCredentials(username, password) {
  localStorage.setItem("chess_user", username || "");
  localStorage.setItem("chess_pass", password || "");
}

function getSavedCredentials() {
  return {
    username: localStorage.getItem("chess_user") || "",
    password: localStorage.getItem("chess_pass") || ""
  };
}

async function ensureSession() {
  try {
    const stats = await apiGet("/get-stats");
    if (stats && stats.ok) return true;
  } catch (e) {
    /* Try auto-login below. */
  }

  const creds = getSavedCredentials();
  if (!creds.username || !creds.password) return false;

  try {
    const login = await apiPost("/login", {
      username: creds.username,
      password: creds.password
    });
    return !!(login && login.ok);
  } catch (e) {
    return false;
  }
}

function applyTheme(theme) {
  if (theme === "dark") document.body.classList.add("dark-mode");
  else document.body.classList.remove("dark-mode");
  localStorage.setItem("chess_theme", theme);
}

function loadSavedTheme() {
  applyTheme(localStorage.getItem("chess_theme") || "light");
}

async function initLoginPage() {
  const loginBtn = document.getElementById("loginBtn");
  const signupBtn = document.getElementById("signupBtn");

  loginBtn.addEventListener("click", async () => {
    const username = document.getElementById("username").value.trim();
    const password = document.getElementById("password").value.trim();
    try {
      const data = await apiPost("/login", { username, password });
      if (!data.ok) {
        setMessage("message", data.error || "Login failed.", false);
        return;
      }
      saveCredentials(username, password);
      saveUsername(data.username);
      go("dashboard.html");
    } catch (e) {
      setMessage("message", "Server not reachable. Run chess_server and open via localhost:8080.", false);
    }
  });

  signupBtn.addEventListener("click", async () => {
    const username = document.getElementById("username").value.trim();
    const password = document.getElementById("password").value.trim();
    try {
      const data = await apiPost("/signup", { username, password });
      if (!data.ok) {
        setMessage("message", data.error || "Signup failed.", false);
        return;
      }
      setMessage("message", "Account created. Now login.", true);
    } catch (e) {
      setMessage("message", "Server not reachable. Run chess_server and open via localhost:8080.", false);
    }
  });
}

async function initDashboardPage() {
  const ok = await ensureSession();
  if (!ok) {
    setMessage("message", "Session expired. Please login again.", false);
    setTimeout(() => go("index.html"), 900);
    return;
  }
  const user = getUsername();
  document.getElementById("welcomeText").textContent = `Welcome, ${user}`;

  document.getElementById("startGameBtn").addEventListener("click", async () => {
    const btn = document.getElementById("startGameBtn");
    btn.disabled = true;
    btn.textContent = "Starting...";
    setMessage("message", "Loading game...", true);
    try {
      const data = await apiPost("/start-game", {});
      if (!data.ok) {
        setMessage("message", data.error || "Could not start game.", false);
        return;
      }
      go("game.html");
    } catch (e) {
      setMessage("message", "Server timeout. Check server terminal and refresh.", false);
    } finally {
      btn.disabled = false;
      btn.textContent = "Start Game";
    }
  });
  document.getElementById("statsBtn").addEventListener("click", () => go("stats.html"));
  document.getElementById("settingsBtn").addEventListener("click", () => go("settings.html"));
  document.getElementById("instructionsBtn").addEventListener("click", () => go("instructions.html"));
  document.getElementById("logoutBtn").addEventListener("click", async () => {
    await apiPost("/logout", {});
    localStorage.removeItem("chess_user");
    go("index.html");
  });
}

function validTargetsForSelected() {
  if (!appState.selected) return [];
  return appState.legalMoves.filter(
    (m) => m.fr === appState.selected.r && m.fc === appState.selected.c
  );
}

function renderBoard() {
  const boardEl = document.getElementById("board");
  boardEl.innerHTML = "";

  const valids = validTargetsForSelected();

  for (let r = 0; r < 8; r++) {
    for (let c = 0; c < 8; c++) {
      const sq = document.createElement("div");
      const isLight = (r + c) % 2 === 0;
      sq.className = `square ${isLight ? "light" : "dark"}`;
      sq.dataset.r = String(r);
      sq.dataset.c = String(c);
      sq.textContent = PIECE_MAP[appState.board[r][c]] || "";

      if (appState.selected && appState.selected.r === r && appState.selected.c === c) {
        sq.classList.add("selected");
      }
      if (valids.some((m) => m.tr === r && m.tc === c)) {
        sq.classList.add("valid");
      }
      if (
        appState.lastMove &&
        ((appState.lastMove.fr === r && appState.lastMove.fc === c) ||
         (appState.lastMove.tr === r && appState.lastMove.tc === c))
      ) {
        sq.classList.add("last-move");
      }

      sq.addEventListener("click", () => onSquareClick(r, c));
      boardEl.appendChild(sq);
    }
  }
}

function renderMoveLog() {
  const list = document.getElementById("moveLogList");
  if (!list) return;
  list.innerHTML = "";
  appState.moveHistory.forEach((mv) => {
    const li = document.createElement("li");
    li.textContent = mv.replace(" ", " -> ");
    list.appendChild(li);
  });
}

async function loadBoard() {
  setMessage("statusText", "Loading board...", true);
  try {
    const data = await apiGet("/get-board");
    if (!data.ok) {
      setMessage("statusText", data.error || "No active game.", false);
      return;
    }
    appState.board = data.board;
    appState.legalMoves = data.legalMoves || [];
    appState.selected = null;
    appState.lastMove = data.lastMove && data.lastMove.valid ? data.lastMove : null;
    appState.moveHistory = data.moveHistory || [];
    document.getElementById("turnText").textContent = `Turn: ${data.turn}`;
    setMessage("statusText", data.status || "normal", true);
    renderBoard();
    renderMoveLog();
  } catch (e) {
    setMessage("statusText", "Server timeout. Please refresh or restart server.", false);
  }
}

async function doAiMoveWithDelay() {
  setMessage("statusText", "AI is thinking...", true);
  await new Promise((resolve) => setTimeout(resolve, 1400));
  try {
    const data = await apiPost("/ai-move", {});
    if (!data.ok) {
      setMessage("statusText", data.error || "AI move failed.", false);
      return;
    }
    appState.board = data.board;
    appState.legalMoves = data.legalMoves || [];
    appState.selected = null;
    appState.lastMove = data.lastMove && data.lastMove.valid ? data.lastMove : null;
    appState.moveHistory = data.moveHistory || [];
    document.getElementById("turnText").textContent = `Turn: ${data.turn}`;
    setMessage("statusText", data.status || "normal", true);
    renderBoard();
    renderMoveLog();
  } catch (e) {
    setMessage("statusText", "Server timeout while AI moved.", false);
  }
}

async function onSquareClick(r, c) {
  if (!appState.board) return;

  if (!appState.selected) {
    const hasMoves = appState.legalMoves.some((m) => m.fr === r && m.fc === c);
    if (hasMoves) {
      appState.selected = { r, c };
      renderBoard();
    }
    return;
  }

  const move = appState.legalMoves.find(
    (m) => m.fr === appState.selected.r && m.fc === appState.selected.c && m.tr === r && m.tc === c
  );

  if (!move) {
    const hasMoves = appState.legalMoves.some((m) => m.fr === r && m.fc === c);
    appState.selected = hasMoves ? { r, c } : null;
    renderBoard();
    return;
  }

  try {
    const data = await apiPost("/move", {
      fr: move.fr, fc: move.fc, tr: move.tr, tc: move.tc
    });
    if (!data.ok) {
      setMessage("statusText", data.error || "Move failed.", false);
      return;
    }

    appState.board = data.board;
    appState.legalMoves = data.legalMoves || [];
    appState.selected = null;
    appState.lastMove = data.lastMove && data.lastMove.valid ? data.lastMove : null;
    appState.moveHistory = data.moveHistory || [];
    document.getElementById("turnText").textContent = `Turn: ${data.turn}`;
    setMessage("statusText", data.status || "normal", true);
    renderBoard();
    renderMoveLog();
    if (data.aiPending) {
      await doAiMoveWithDelay();
    }
  } catch (e) {
    setMessage("statusText", "Server timeout while moving piece.", false);
  }
}

async function initGamePage() {
  const ok = await ensureSession();
  if (!ok) {
    setMessage("statusText", "Session expired. Please login again.", false);
    setTimeout(() => go("index.html"), 900);
    return;
  }
  document.getElementById("refreshBtn").addEventListener("click", loadBoard);
  document.getElementById("backDashboardBtn").addEventListener("click", () => go("dashboard.html"));
  /* Requirement: on refresh/open this page, start a fresh game automatically. */
  try {
    const res = await apiPost("/start-game", {});
    if (!res.ok) setMessage("statusText", res.error || "Could not start new game.", false);
  } catch (e) {
    setMessage("statusText", "Could not start game. Is server running?", false);
  }
  await loadBoard();
}

async function initStatsPage() {
  const ok = await ensureSession();
  if (!ok) {
    go("index.html");
    return;
  }
  const data = await apiGet("/get-stats");
  if (data.ok) {
    document.getElementById("statsUsername").textContent = `Player: ${data.username}`;
    document.getElementById("gamesPlayed").textContent = data.gamesPlayed;
    document.getElementById("wins").textContent = data.wins;
    document.getElementById("losses").textContent = data.losses;
    document.getElementById("draws").textContent = data.draws;
  }
  document.getElementById("backBtn").addEventListener("click", () => go("dashboard.html"));
}

async function initSettingsPage() {
  const keybindingEl = document.getElementById("keybindingStyle");
  const boardStyleEl = document.getElementById("boardStyle");
  const themeEl = document.getElementById("themeMode");
  const saveBtn = document.getElementById("saveSettingsBtn");
  const resetBtn = document.getElementById("resetStatsBtn");
  const backBtn = document.getElementById("backBtn");

  /* Theme should work even if backend is down or user is not logged in. */
  themeEl.value = localStorage.getItem("chess_theme") || "light";
  themeEl.addEventListener("change", (e) => {
    applyTheme(e.target.value);
    setMessage("message", `Theme changed to ${e.target.value}.`, true);
  });

  const ok = await ensureSession();
  if (!ok) {
    setMessage("message", "Please login first. Theme still works locally.", false);
    saveBtn.disabled = true;
    resetBtn.disabled = true;
  }

  saveBtn.addEventListener("click", async () => {
    try {
      const keybinding_style = Number(keybindingEl.value);
      const board_style = Number(boardStyleEl.value);
      const data = await apiPost("/settings", { keybinding_style, board_style });
      if (!data.ok) return setMessage("message", data.error || "Could not save settings.", false);
      setMessage("message", "Settings saved.", true);
    } catch (e) {
      setMessage("message", "Could not save settings. Check server/login.", false);
    }
  });

  resetBtn.addEventListener("click", async () => {
    const confirmed = window.confirm("Reset all stats (games, wins, losses, draws)?");
    if (!confirmed) return;
    try {
      const data = await apiPost("/reset-stats", {});
      if (!data.ok) return setMessage("message", data.error || "Could not reset stats.", false);
      setMessage("message", "Stats reset successfully.", true);
    } catch (e) {
      setMessage("message", "Server not reachable for reset.", false);
    }
  });

  backBtn.addEventListener("click", () => go("dashboard.html"));
}

function initInstructionsPage() {
  document.getElementById("backBtn").addEventListener("click", () => go("dashboard.html"));
}

window.addEventListener("DOMContentLoaded", async () => {
  loadSavedTheme();
  if (window.location.protocol === "file:") {
    alert("Open the app through the C server URL: http://localhost:8080/index.html");
  }
  const page = currentPage();
  if (page === "login") await initLoginPage();
  else if (page === "dashboard") await initDashboardPage();
  else if (page === "game") await initGamePage();
  else if (page === "stats") await initStatsPage();
  else if (page === "settings") await initSettingsPage();
  else if (page === "instructions") initInstructionsPage();
});

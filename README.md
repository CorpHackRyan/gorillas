# 🐒 Gorillas in C
## 🎮 Terminal Game Engine Using ncurses
This project is the beginning of a terminal-based clone of *Gorillas* — written in pure C using the ncurses library. You control two on-screen characters:

- **Player 1:** `@`
- **Player 2:** `#`

Both can be drawn and moved around a real-time ncurses game world.  
This is your foundation for a full physics + projectile game.

---

## ✨ Features
### ✔️ ncurses-Based Movement Engine
- Move **Player 1 (`@`)** using arrow keys
- Characters erase and redraw smoothly
- Prevents movement outside the terminal bounds
- Arrow keys enabled with `keypad(stdscr, TRUE)`
- Cursor hidden for a clean UI
- Real-time key reading via `getch()`

---

## 📜 How the Code Works
### 🔧 Initialization
```c
initscr();        // Start ncurses mode
curs_set(0);      // Hide cursor
keypad(stdscr, TRUE); // Enable arrow keys
noecho();         // Don't echo keypresses
y = row (top → bottom)
x = column (left → right)
mvaddch(player1_y, player1_x, '@');

## 🧠 Ternary Operator
You use this to stop `@` from moving offscreen:

```c
player1_y = (player1_y > 0) ? player1_y - 1 : player1_y;
```

## This is Stage 1 of my game engine.
### Future features include:

- 🏙️ Random city skyline
- 🌬️ Wind affecting projectiles
- 🍌 Banana physics (angle + velocity)
- 💥 Explosion collision detection
- 👥 Two-player mode (@ vs #)

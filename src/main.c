#include <stdio.h>
#include <ncurses.h>


int main() {
    int ch;
    int player1_x = 10, player1_y = 20;
    int player2_x = 70, player2_y = 30;
    char player1 = '@';
    char player2 = '#';

    // Documentation below of STUFF used in C mostly ncurses
    
    // initscr()                — initializes ncurses
    // noecho()                 — disables automatic key printing
    // curs_set(0)              — hides the blinking cursor
    // mvprintw(5, 10, "text")  — prints “Hello, Gorillas!” at a specific location
    // refresh()                — updates the terminal
    // getch()                  — waits for you to press any key
    // endwin()                 — restores the terminal to normal mode
    // keypad(stdscr, TRUE);    — enable arrow keys


    // LINES and COLS documentation
    // LINES (all caps) is a predefined global macro that gives you the number of rows (height) of the terminal window.
    // ------------------------------------------------------------
    // LINES  -> total number of rows (height) of the terminal
    // COLS   -> total number of columns (width) of the terminal
    //
    // Both are automatically set by ncurses after initscr() is called.
    // Example usage:
    //      if (player_y < LINES - 1) player_y++;  // prevents moving off bottom
    //      if (player_x < COLS - 1)  player_x++;  // prevents moving off right
    //
    // Notes:
    // - Terminal coordinates are zero-indexed: (0,0) is top-left
    // - Use LINES for vertical bounds and COLS for horizontal bounds
    // ------------------------------------------------------------


    initscr();
    curs_set(0); //Hide the cursor
    keypad(stdscr, TRUE); // enable arrow keys

    mvprintw(1, 5, "Welcome to Gorillas in C!"); 
    mvaddch(player1_y, player1_x, player1); //Draw Player 1
    refresh();

    noecho(); //Disable automatic key printing

    while ((ch = getch()) != 'q') { //Wait for user to press 'q' to quit
        
        mvaddch(player1_y, player1_x, ' '); //Erase Player 1

        switch (ch) {
            case KEY_UP:
                player1_y = (player1_y > 0) ? player1_y - 1 : player1_y;
                //if (player1_y > 0) player1_y++;
                break;
            case KEY_DOWN:
                player1_y = (player1_y < LINES - 1) ? player1_y + 1 : player1_y;
                break;
            case KEY_LEFT:
                player1_x = (player1_x > 0) ? player1_x - 1 : player1_x;
                break;
            case KEY_RIGHT:
                player1_x = (player1_x < COLS - 1) ? player1_x + 1 : player1_x;
                break;
            default:
                break;
        }
        mvaddch(player1_y, player1_x, player1); //Draw Player 1
        mvprintw(12, 25, "You pressed: %c", ch); //Print the pressed key
        refresh();
    }

    endwin(); //Restore normal terminal behavior
    printf("\n\nDebug build successful.\n");
    return 0;
    
}

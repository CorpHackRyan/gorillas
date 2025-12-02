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


#include <stdio.h>
#include <ncurses.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>

const char C_STAR = '*';



void animate_top_border(const char* blink_text, int blink_row, int blink_col) 
{
    int top_x = 0;  // Current moving star position
    int ch = ERR;   // key press variable
    bool visible = true;
    
    timeout(0);     // make getch() non-blocking to allow animation

    // Draw static borders first
    for (int x = 0; x < COLS; x++) {
        mvaddch(0, x, C_STAR);         // top border
        mvaddch(LINES - 1, x, C_STAR); // bottom border
    }
    for (int y = 1; y < LINES - 1; y++) {
        mvaddch(y, 0, C_STAR);         // left border
        mvaddch(y, COLS - 1, C_STAR);  // right border
    }

    refresh();

    // Animation loop
    while ((ch  = getch()) == ERR) {        // Run until any key is pressed
        mvaddch(0, top_x, C_STAR);          // Erase the moving star by redrawing the static border at old position
        top_x = (top_x + 1) % COLS;         // Move the star
        mvaddch(0, top_x, '@');             // Draw the moving star

        if (visible) {
            mvprintw(blink_row, blink_col, "%s", blink_text);  // Display the blinking text 
        } else {
            //mvprintw(blink_row, blink_col, " ");  // Erase the blinking text
            for (int i = 0; i < strlen(blink_text); i++) {
                mvaddch(blink_row, blink_col + i, ' ');
            }
        }
        visible = !visible;                 // Toggle visibility  


        refresh();                          // Refresh the screen
        napms(150);                          // Sleep for 250ms        
    }

    timeout(-1);                            // restore default timeout
}



// ---------------------------------------------------------------------------
// Function: intro_screen()
// Description:
//      Displays a centered, large ASCII-style title "Gorillas in C" and waits
//      for the user to press ANY key before continuing.
// ---------------------------------------------------------------------------
void intro_screen() {
    clear();
    

    // Large ASCII title (double-sized text effect)
    // const char* means the pointer can change, but the characters it points to cannot.
    // We use this for read-only strings (like literals) to prevent accidental modification.
    
    // const char* = pointer to constant chars - Safe for string literals and functions that should NOT modify the input string.


    // Your block text banner
    const char *banner[] = {
        " ██████   ██████  ██████  ██  ██      ██         █     █████",
        "██       ██    ██ ██   ██ ██  ██      ██       █   █   ██   ",
        "██   ███ ██    ██ ██████  ██  ██      ██      █     █  █████",
        "██    ██ ██    ██ ██   ██ ██  ██      ██      █ ███ █     ██",
        " ██████   ██████  ██   ██ ██  ███████ ███████ █     █  █████"
    };

    int banner_rows = sizeof(banner) / sizeof(banner[0]);       // number of lines in the banner
    int banner_width = 60;                                      // number of characters in the longest line

    // Compute starting position to center the banner
    int start_row = (LINES - banner_rows) / 2;
    int start_col = (COLS - banner_width) / 2;

    // Print each line centered
    for (int i = 0; i < banner_rows; i++) {
        mvaddstr(start_row + i, start_col, banner[i]);
    }
    
    // Print the prompt below the banner
    // mvprintw() - This is an ncurses function that moves the cursor to a specific position (y, x) and prints the string "text".
    
    char *intro_txt1 = "Copyright (C) CorpHackRyan Corporation 2025";
    char *intro_txt2 = "our mission is to hit your opponent with the exploding";
    char *intro_txt3 = "banana by varying the angle and power of your throw, taking";
    char *intro_txt4 = "into account wind speed, gravity, and the city skyline.";
    char *intro_txt5 = "The wind speed is shown by a directional arrow at the bottom";
    char *intro_txt6 = "of the playing field, its length relative to its strength.";
    char *intro_txt7 = "Press any key to continue";

    mvprintw(start_row + banner_rows + 1, (COLS - strlen(intro_txt1)) / 2, "%s", intro_txt1);
    mvprintw(start_row + banner_rows + 2, (COLS - strlen(intro_txt2)) / 2, "%s", intro_txt2);
    mvprintw(start_row + banner_rows + 3, (COLS - strlen(intro_txt3)) / 2, "%s", intro_txt3);
    mvprintw(start_row + banner_rows + 4, (COLS - strlen(intro_txt4)) / 2, "%s", intro_txt4);
    mvprintw(start_row + banner_rows + 5, (COLS - strlen(intro_txt5)) / 2, "%s", intro_txt5);
    mvprintw(start_row + banner_rows + 6, (COLS - strlen(intro_txt6)) / 2, "%s", intro_txt6);
    //mvprintw(start_row + banner_rows + 8, (COLS - strlen(intro_txt7)) / 2, "%s", intro_txt7); 
   
    // Compute position for the blinking cursor
    // int blink_row = start_row + banner_rows + 8;
    // int blink_col = (COLS - strlen(intro_txt7)) / 2;

    animate_top_border(intro_txt7, start_row + banner_rows + 8, (COLS - strlen(intro_txt7)) / 2);   // Animate the top border

    //refresh();
    //getch();  // Wait for ANY KEY
    clear();
    refresh();
}


int main() {
    setlocale(LC_ALL, "");
    int ch;
    int player1_x = 10, player1_y = 20;
    int player2_x = 70, player2_y = 30;
    char* player1 = '>';                   // If you want to print a whole string, you need mvprintw() instead of mvaddch():
    char player2 = '#';

    initscr();
    curs_set(0);            // Hide the cursor
    keypad(stdscr, TRUE);   // enable arrow keys


    // Run the intro screen first
    intro_screen();         // Display the intro screen
    


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
        mvaddch(player1_y, player1_x, player1);  // Draw Player 1
        mvprintw(0, 0, "You pressed: %c", ch); // Print the pressed key
        refresh();
    }

    endwin(); //Restore normal terminal behavior
    printf("\n\nDebug build successful.\n");
    return 0;
    
}

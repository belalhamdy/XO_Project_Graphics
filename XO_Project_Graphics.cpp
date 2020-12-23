#include <stdio.h>
#include <windows.h>
#include "CC212SGL.h"

#pragma comment(lib, "CC212SGL.lib")
/*
constants for application
*/
#define GRID_SIZE 3
#define EMPTY -1
#define TIE 0
#define GAME_IN_PROCESS GRID_SIZE*4
#define X 1
#define O 0
#define FONT_SIZE 30
#define DRAW_SIZE 200
#define PADDING 30
#define MARGIN 0
#define UNIT (DRAW_SIZE + MARGIN)
#define RESET_BUTTON 'R'

// draws the whole board on the screen
void draw_board(CC212SGL& g, int grid[GRID_SIZE][GRID_SIZE]);

// draws the borders of the board (lines)
void draw_borders(CC212SGL& g);

// draws the line of the winning row or column or diagonal
void draw_winning_line(CC212SGL& g, int val);

// draws X on the screen given the x,y coordinates
void draw_X(CC212SGL& g, int x, int y);

// draws O on the screen given the x,y coordinates
void draw_O(CC212SGL& g, int x, int y);

// draws index of the unit on the screen given the x,y coordinates
void draw_Empty(CC212SGL& g, int x, int y, int val);

// prints to player whether to play 1 vs 1 or || 1 vs pc
void draw_first_input_screen(CC212SGL& g);

// takes input from the player whether to play 1 vs 1 or || 1 vs pc
int get_from_first_input_screen(CC212SGL& g);

// prints on the screen which player should play now
void draw_player_turn_text(CC212SGL& g, const char* name);

// takes input from the player where to play
void draw_player_won_text(CC212SGL& g, const char* name);

// prints to player whether to play again or not
void draw_do_you_want_to_play_again(CC212SGL& g);

// takes input from the player whether to play again or not
int get_from_do_you_want_to_play_again(CC212SGL& g);

// takes number from 1 to 9 of the playing index also you can reset game by pressing r
int get_player_turn(CC212SGL& g); 

/*
takes the play of the current player and checks if the cell is empty or not 
if it is empty it updates the grid with the mark
*/
int update_grid(int grid[GRID_SIZE][GRID_SIZE],int idx,int mark);
 

/*
 Returns:
 * 0 TIE -> no one wins and TIE
 * +ve number <= GRID_SIZE(3) -> index of row that wins
 * -ve number <= GRID_SIZE(3) -> index of col that wins
 * GRID_SIZE + 1 (4) -> diagonal that goes from left to right wins
 * - (GRID_SIZE + 1) (-4) -> diagonal that goes from right to left wins
 * Anything Else -> GAME_IN_PROCESS (GRID_SIZE * 4)
 */
int check_win(int grid[GRID_SIZE][GRID_SIZE]);

// Finds for the computer the best spot to play
int computer_turn(int grid[GRID_SIZE][GRID_SIZE], int computer_mark);
int main()
{
	CC212SGL g;

	g.setup();

	g.setFullScreenMode();
	g.hideCursor();


    while (1) {
        g.beginDraw();
        draw_first_input_screen(g);
        g.endDraw();
        int players = get_from_first_input_screen(g);
        if (players == 0)
            break;

        const char* first_player_name = "X";
        int first_player_mark = X;

        const char* second_player_name = "O";
        int second_player_mark = O;

        const char* player_won = "";

        int grid[GRID_SIZE][GRID_SIZE] = {
        {EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY},
        {EMPTY, EMPTY, EMPTY} };

        int reset = 0;

        // start the game
        while (check_win(grid) == GAME_IN_PROCESS) {
            int play_idx;

            g.beginDraw();
            draw_board(g, grid);
            draw_player_turn_text(g, first_player_name);
            g.endDraw();

            while (1) {
                play_idx = get_player_turn(g);

                // reset is pressed
                if (play_idx == 0) {
                    reset = 1;
                    break;
                }
                if (update_grid(grid, play_idx - 1, first_player_mark))
                    break;
            }
            if (reset) break;
            if (check_win(grid) != GAME_IN_PROCESS) {
                player_won = first_player_name;
                break;
            }

            g.beginDraw();
            draw_board(g, grid);
            draw_player_turn_text(g, second_player_name);
            g.endDraw();

            if (players == 1)
                computer_turn(grid, second_player_mark);
            else {
                while (1) {
                    play_idx = get_player_turn(g);

                    // reset is pressed
                    if (play_idx == 0) {
                        reset = 1;
                        break;
                    }
                    if (update_grid(grid, play_idx - 1, second_player_mark))
                        break;
                }
            }
            if (reset) break;
            if (check_win(grid) != GAME_IN_PROCESS) {
                player_won = second_player_name;
                break;
            }
            
        }
        if (reset)
            continue;
        if (check_win(grid) == TIE)
            player_won = "TIE";
        g.beginDraw();
        draw_board(g, grid);
        draw_player_won_text(g, player_won);
        draw_winning_line(g, check_win(grid));
        draw_do_you_want_to_play_again(g);
        g.endDraw();

        int play_again = get_from_do_you_want_to_play_again(g);
        if (play_again == 0) break;

    }

	_getch();
    return 0;

}

// Checks if the game ended tie or any one won or still in progress
int check_win(int grid[GRID_SIZE][GRID_SIZE]) {
    int i, j;
    int row_wins, col_wins, left_diagonal_wins = 1, right_diagonal_wins = 1;
    for (i = 0; i < GRID_SIZE; ++i) {
        row_wins = (grid[i][0] != EMPTY);
        col_wins = (grid[0][i] != EMPTY);
        for (j = 1; j < GRID_SIZE; ++j) {
            row_wins &= (grid[i][j - 1] == grid[i][j]);
            col_wins &= (grid[j - 1][i] == grid[j][i]);

            if (i == j) {
                left_diagonal_wins &= (grid[0][0] == grid[i][j] && grid[0][0] != -1);
                right_diagonal_wins &= (grid[0][GRID_SIZE - 1] == grid[i][GRID_SIZE - j - 1] &&
                    grid[0][GRID_SIZE - 1] != -1);
            }
        }
        if (row_wins) return i + 1;
        else if (col_wins) return -(i + 1);
    }
    if (left_diagonal_wins) return GRID_SIZE + 1;
    else if (right_diagonal_wins) return -(GRID_SIZE + 1);

    int empty_cells = 0;
    for (i = 0; i < GRID_SIZE; ++i)
        for (j = 0; j < GRID_SIZE; ++j)
            empty_cells += (grid[i][j] == EMPTY);

    if (empty_cells == 0) return TIE;

    return GAME_IN_PROCESS; // A unique number that defines that the game has not ended yet
}

/*
* First try to find a row or column that contains 2 of `computer_mark` and there is an empty cell so you can win
* If not found try to find a row or column that contains 2 of `opponent_mark` and there is an empty cell so you may lose
* If not find a row or column or diagonal that does not contain the opponent mark
* If not find any empty cell
*/
int computer_turn(int grid[GRID_SIZE][GRID_SIZE], int computer_mark) {
    const int row = 0, column = 1, diagonal = 2; // First Dimension Size (types)
    const int number_of_my_marks = 0, number_of_empty_cells = 1; // Third Dimension Size
    const int first_dimension_size = 3, second_dimension_size = GRID_SIZE, third_dimension_size = 2;
    int grid_analysis[first_dimension_size][second_dimension_size][third_dimension_size];

    // initialize the grid
    int i, j, k;
    for (i = 0; i < first_dimension_size; ++i)
        for (j = 0; j < second_dimension_size; ++j)
            for (k = 0; k < third_dimension_size; ++k)
                grid_analysis[i][j][k] = 0;

    // study the grid
    for (i = 0; i < GRID_SIZE; ++i) {
        for (j = 0; j < GRID_SIZE; ++j) {
            if (grid[i][j] == computer_mark) grid_analysis[row][i][number_of_my_marks]++;
            else if (grid[i][j] == EMPTY) grid_analysis[row][i][number_of_empty_cells]++;

            if (grid[j][i] == computer_mark) grid_analysis[column][i][number_of_my_marks]++;
            else if (grid[j][i] == EMPTY) grid_analysis[column][i][number_of_empty_cells]++;

            if (i == j) {
                if (grid[i][j] == computer_mark)
                    grid_analysis[diagonal][0][number_of_my_marks]++; // 0 means the left to right diagonal
                else if (grid[i][j] == EMPTY) grid_analysis[diagonal][0][number_of_empty_cells]++;

                int rj = GRID_SIZE - i - 1;
                if (grid[i][rj] == computer_mark)
                    grid_analysis[diagonal][1][number_of_my_marks]++; // 1 means the right to left diagonal
                else if (grid[i][rj] == EMPTY) grid_analysis[diagonal][1][number_of_empty_cells]++;
            }
        }
    }

    // Find a row or a column or diagonal that contains your mark
    int found_idx = -1, found_type, found_importance = 99999, current_importance;
    for (i = 0; i < second_dimension_size; ++i) {
        // Find a way to win
        current_importance = 1;
        if (found_importance > current_importance) {
            if (grid_analysis[row][i][number_of_my_marks] == GRID_SIZE - 1 &&
                grid_analysis[row][i][number_of_empty_cells] == 1) {
                found_idx = i;
                found_type = row;
                found_importance = current_importance;
            }
            else if (grid_analysis[column][i][number_of_my_marks] == GRID_SIZE - 1 &&
                grid_analysis[column][i][number_of_empty_cells] == 1) {
                found_idx = i;
                found_type = column;
                found_importance = current_importance;
            }
            else if (grid_analysis[diagonal][i][number_of_my_marks] == GRID_SIZE - 1 &&
                grid_analysis[diagonal][i][number_of_empty_cells] == 1) {
                found_idx = i;
                found_type = diagonal;
                found_importance = current_importance;
            }
        }

        // Find a way to prevent your loss
        current_importance = 2;
        if (found_importance > current_importance) {
            if (grid_analysis[row][i][number_of_my_marks] == 0 && grid_analysis[row][i][number_of_empty_cells] == 1) {
                found_idx = i;
                found_type = row;
                found_importance = current_importance;
            }
            else if (grid_analysis[column][i][number_of_my_marks] == 0 &&
                grid_analysis[column][i][number_of_empty_cells] == 1) {
                found_idx = i;
                found_type = column;
                found_importance = current_importance;
            }
            else if (grid_analysis[diagonal][i][number_of_my_marks] == 0 &&
                grid_analysis[diagonal][i][number_of_empty_cells] == 1) {
                found_idx = i;
                found_type = diagonal;
                found_importance = current_importance;
            }
        }
        // choose the row or column that has 0 of opponent mark
        current_importance = 3;
        if (found_importance > current_importance) {
            if (grid_analysis[row][i][number_of_my_marks] + grid_analysis[row][i][number_of_empty_cells] == GRID_SIZE) {
                found_idx = i;
                found_type = row;
                found_importance = current_importance;
            }
            else if (grid_analysis[column][i][number_of_my_marks] + grid_analysis[column][i][number_of_empty_cells] ==
                GRID_SIZE) {
                found_idx = i;
                found_type = column;
                found_importance = current_importance;
            }
            else if (grid_analysis[diagonal][i][number_of_my_marks] +
                grid_analysis[diagonal][i][number_of_empty_cells] == GRID_SIZE) {
                found_idx = i;
                found_type = diagonal;
                found_importance = current_importance;
            }
        }
        // Find any cell to fill it
        current_importance = 4;
        if (found_importance > current_importance) {
            if (grid[GRID_SIZE / 2][GRID_SIZE / 2] == EMPTY) {
                found_idx = GRID_SIZE / 2;
                found_type = row;
                found_importance = current_importance;
            }
            else if (grid_analysis[row][i][number_of_empty_cells]) {
                found_idx = i;
                found_type = row;
                found_importance = current_importance;
            }
        }

    }

    if (found_idx == -1) return 0;

    int ti = -1, tj = -1;
    if (found_type == row) {
        ti = found_idx;
        for (i = 0; i < GRID_SIZE; ++i) {
            if (grid[found_idx][i] == EMPTY) tj = i;
        }
    }
    else if (found_type == column) {
        tj = found_idx;
        for (i = 0; i < GRID_SIZE; ++i) {
            if (grid[i][found_idx] == EMPTY) ti = i;
        }
    }
    else if (found_type == diagonal) {
        if (found_idx == 0)
            for (i = 0; i < GRID_SIZE; ++i) {
                if (grid[i][i] == EMPTY) ti = tj = i;
            }
        else
            for (i = 0; i < GRID_SIZE; ++i) {
                if (grid[i][GRID_SIZE - i - 1] == EMPTY) {
                    ti = i;
                    tj = GRID_SIZE - i - 1;
                }
            }
    }

    if (ti == -1 || tj == -1) return 0;
    grid[ti][tj] = computer_mark;
    return 1;
}

int update_grid(int grid[GRID_SIZE][GRID_SIZE], int idx, int mark)
{
    int i = idx / 3;
    int j = idx % 3;
    if (grid[i][j] != EMPTY) return 0;
    grid[i][j] = mark;
    return 1;
}

void draw_board(CC212SGL& g, int grid[GRID_SIZE][GRID_SIZE]) {
    // first draw the borders of the game
    draw_borders(g);

    g.setFontSizeAndBoldness(DRAW_SIZE, 5);
    int i, j;
    for (i = 0; i < GRID_SIZE; ++i) {
        for (j = 0; j < GRID_SIZE; ++j) {
            int position_x = (j * UNIT) + UNIT / 12;
            int position_y = (i * UNIT) + UNIT / 6;
            if (grid[i][j] == X) draw_X(g, position_x, position_y);
            else if (grid[i][j] == O) draw_O(g, position_x, position_y);
            else if (grid[i][j] == EMPTY) draw_Empty(g, position_x, position_y, i * GRID_SIZE + j + 1);

        }
    }

}
void draw_X(CC212SGL& g, int x, int y) {
    g.setDrawingColor(COLORS::LIME);
    g.drawText(x, y, "X");
}
void draw_O(CC212SGL& g, int x, int y) {
    g.setDrawingColor(COLORS::RED);
    g.drawText(x, y, "O");
}
void draw_Empty(CC212SGL& g, int x, int y, int val) {
    g.setDrawingColor(COLORS::YELLOW);
    char c[20];
    sprintf_s(c, "%d", val);
    g.drawText(x, y, c);
}
void draw_borders(CC212SGL& g) {
    g.setDrawingColor(COLORS::WHITE);
    // Vertical Lines
    g.drawLine(UNIT, 0, UNIT, 3 * UNIT);
    g.drawLine(2 * UNIT, 0, 2 * UNIT, 3 * UNIT);

    // Horizontal Lines
    g.drawLine(0, UNIT, 3 * UNIT, UNIT);
    g.drawLine(0, 2 * UNIT, 3 * UNIT, 2 * UNIT);
}
void draw_winning_line(CC212SGL& g, int val) {
    g.setDrawingColor(COLORS::CYAN);
    g.setDrawingThickness(5);
    int startX = 0, startY = 0, endX = 0, endY = 0;
    int padding = UNIT / 2;
    // column -> vertical
    if (val < 0 && val >= -GRID_SIZE) {
        val = (-1 * val) - 1;
        startX = endX = val * UNIT + padding;
        endY = 3 * UNIT;
    }
    // row -> horizontal
    else if (val > 0 && val <= GRID_SIZE) {
        val = val - 1;
        endX = 3 * UNIT;
        startY = endY = val * UNIT + padding;
    }
    // Left to right diagonal
    else if (val == (GRID_SIZE + 1)) {
        endX = endY = 3 * UNIT;
    }
    // right to left diagonal
    else if (val == -(GRID_SIZE + 1)) {
        startX = endY = 3 * UNIT;
    }

    else return;

    g.drawLine(startX, startY, endX, endY);
}

void draw_first_input_screen(CC212SGL& g) {
    g.setFontSizeAndBoldness(FONT_SIZE,5);
    g.setDrawingColor(COLORS::WHITE);

    const char* text = "Welcome to XO game\n1- Play vs computer\n2- Play vs another player\n0- Exit\n\npress 'r' to reset the game anytime.";
    g.drawText(PADDING, 5, text);
}
void draw_player_turn_text(CC212SGL& g, const char* name)
{
    g.setFontSizeAndBoldness(FONT_SIZE, 5);
    g.setDrawingColor(COLORS::WHITE);

    char text[1000] = {};
    strcpy(text, name);
    strcat(text, " turn");
    g.drawText(PADDING, 3 * UNIT + 50, text);
}
void draw_player_won_text(CC212SGL& g, const char* name)
{
    g.setFontSizeAndBoldness(FONT_SIZE, 5);
    g.setDrawingColor(COLORS::MAGENTA);

    char text[1000] = {};
    strcat(text, name);
    if(name != "TIE")
        strcat(text, " Won");
    g.drawText(PADDING, 3 * UNIT + 50, text);
}

void draw_do_you_want_to_play_again(CC212SGL& g)
{
    g.setFontSizeAndBoldness(FONT_SIZE, 5);
    g.setDrawingColor(COLORS::WHITE);

    const char* text = "Do you want to play again?\n1- YES\n2- NO";
    g.drawText(PADDING, 3 * UNIT + 150, text);
}

// 1 -> yes , 2 -> no , 'r' -> reset
int get_from_do_you_want_to_play_again(CC212SGL& g)
{
    char input;
    while (1) {
        if (_kbhit()) //Non Blocking Call: waits for input from user
        {
            input = _getch();	//Blocking Call

            if (GetAsyncKeyState('1') || GetAsyncKeyState(RESET_BUTTON))
                return 1;
            if (GetAsyncKeyState('2'))
                return 0;
        }
    }
    return 0;
}

int get_player_turn(CC212SGL& g)
{
    char input;
    while (1) {
        if (_kbhit()) //Non Blocking Call: waits for input from user
        {
            input = _getch();	//Blocking Call

            if (GetAsyncKeyState('1'))
                return 1;
            if (GetAsyncKeyState('2'))
                return 2;
            if (GetAsyncKeyState('3'))
                return 3;
            if (GetAsyncKeyState('4'))
                return 4;
            if (GetAsyncKeyState('5'))
                return 5;
            if (GetAsyncKeyState('6'))
                return 6;
            if (GetAsyncKeyState('7'))
                return 7;
            if (GetAsyncKeyState('8'))
                return 8;
            if (GetAsyncKeyState('9'))
                return 9;
            if (GetAsyncKeyState(RESET_BUTTON))
                return 0;
        }
    }
    return 0;
}

int get_from_first_input_screen(CC212SGL& g) {
    char input;
    while (1) {
        if (_kbhit()) //Non Blocking Call: waits for input from user
        {
            input = _getch();	//Blocking Call

            if (GetAsyncKeyState('1'))
                return 1;
            if (GetAsyncKeyState('2'))
                return 2;
            if (GetAsyncKeyState('0'))
                return 0;
        }
    }
    return 0;
}



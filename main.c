#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>

bool GameOver = false;

int CurrentPiece = 0;
int CurrentRotation = 0;
int CurrentX = 4;
int CurrentY = 0;
char userInput;
bool screen[20][11];

bool Field[20][11] = {0};
 // rototation = i = 12 + y - (x*4)
 // rot anticlockwise 3 + y + (x*4)
const bool tetrominos[7][4][4] = {
    {0,0,1,0,
     0,0,1,0,
     0,0,1,0,
     0,0,1,0},
    {0,0,1,0,
     0,1,1,0,
     0,1,0,0,
     0,0,0,0},
    {0,1,0,0,
     0,1,1,0,
     0,0,1,0,
     0,0,0,0},
    {0,0,1,0,
     0,0,1,0,
     0,1,1,0,
     0,0,0,0},
    {0,1,0,0,
     0,1,0,0,
     0,1,1,0,
     0,0,0,0},
    {0,0,1,0,
     0,1,1,0,
     0,0,1,0,
     0,0,0,0},
    {0,0,0,0,
     0,1,1,0,
     0,1,1,0,
     0,0,0,0},
};

char getch(void) {
    struct termios oldt, newt;
    char ch;
    int oldf;

    // Get current terminal attributes
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Set terminal to raw mode
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Set non-blocking input
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    // Try to get input
    ch = getchar();

    // Restore blocking mode
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    
    // Restore original terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

int Rotate(int px, int py, int r) {
    switch (r % 4) {
        case 0: return py*4+px;
        case 1: return 12+py-px*4;
        case 2: return 15-py*4-px;
        case 3: return 3 - py + px*4;
    }
    return 0;
};

bool DoesPieceFit(int Tetromino, int Rototation, int PosX, int PosY) {
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            int pi = Rotate(px, py, Rototation);
            if (tetrominos[Tetromino][pi / 4][pi % 4] != 0) {
                int fieldX = PosX + px;
                int fieldY = PosY + py;
                if (fieldX < 0 || fieldX >= 11 || fieldY < 0 || fieldY >= 20) {
                    return false;
                }

                if (Field[fieldY][fieldX] != 0) {
                    return false;
        }}}}
    return true;
    };

void removeLines() {
    for (int i = 0; i < 20; i++) {
        // Check if the line is completely filled
        bool lineFilled = true;
        for (int j = 0; j < 11; j++) {
            if (Field[i][j] == 0) {
                lineFilled = false;
                break;
            }
        }

        // If the line is filled, remove it
        if (lineFilled) {
            // Shift all lines above down by one
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < 11; j++) {
                    Field[k][j] = Field[k - 1][j];
                }
            }
            // Clear the top line
            for (int j = 0; j < 11; j++) {
                Field[0][j] = 0;
            }
        }
    }
}

int main() {
    char input;
    while (!GameOver) {
    usleep(500000);
    // timing
    input = getch();
    switch (input) {
    case 'a': case 'A': if (DoesPieceFit(CurrentPiece,  CurrentRotation,  CurrentX-1,  CurrentY)) CurrentX--; break;
    case 's': case 'S': if (DoesPieceFit(CurrentPiece, CurrentRotation, CurrentX, CurrentY+1)) CurrentY++; break;
    case 'd': case 'D': if (DoesPieceFit(CurrentPiece,  CurrentRotation,  CurrentX+1,  CurrentY)) CurrentX++; break;
    case 'q': case 'Q': if (DoesPieceFit(CurrentPiece,  CurrentRotation + 3,  CurrentX,  CurrentY)) CurrentRotation += 3; break;
    case 'e': case 'E': if (DoesPieceFit(CurrentPiece,  CurrentRotation + 1,  CurrentX,  CurrentY)) CurrentRotation++; break;
    }
    GameOver = false;
        if (DoesPieceFit(CurrentPiece, CurrentRotation, CurrentX, CurrentY + 1)) {
            CurrentY++;
        } else {
            for (int px = 0; px < 4; px++) {
                for (int py = 0; py < 4; py++) {
                    if (tetrominos[CurrentPiece][py][px]) {
                        int pi = Rotate(px, py, CurrentRotation);
                        Field[CurrentY + (pi / 4)][CurrentX + (pi % 4)] = 1;
                    }
                }
            }
            CurrentX = 6;
            CurrentY = 0;
            CurrentPiece = rand() % 7;
            CurrentRotation = 0;
        }
        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 11; j++) {
                screen[i][j] = Field[i][j];
            }
        }
        for (int px = 0; px < 4; px++) {
            for (int py = 0; py < 4; py++) {
                if (tetrominos[CurrentPiece][py][px]) {
                    int pi = Rotate(px, py, CurrentRotation);
                    screen[CurrentY + (pi / 4)][CurrentX + (pi % 4)] = 1;
                }
            }
        }
    //removeLines();
    printf("\033[H\033[J");
    for (int i = 0; i < 20; i++) {
        printf("|");
        for (int j = 0; j < 11; j++) {
            if (screen[i][j] == 0) {
                printf(".");
            } else {
                printf("\u25A0");
            }
        }
        printf("|");
        printf("\n");
    }
    printf("\\-----------/\n");
    fflush(stdout);
    };
    printf("game over your score is %i\n", 100);
    return 0;
}

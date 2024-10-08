#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

int CurrentPiece = 0;
int CurrentRotation = 0;
int CurrentX = 4;
int CurrentY = 0;
char userInput;
bool screen[20][11];
int TimeCounter = 0;
int Score = 0;

bool Field[20][11] = {0};
// rototation = i = 12 + y - (x*4)
// rot anticlockwise 3 + y + (x*4)
const bool tetrominos[7][4][4][4] = { // tetromino rototation py px
    {{0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
     {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0},
     {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}},
    {
        {0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
        {0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    },
    {
        {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0},
    },
    {
        {0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0},
    }};

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

bool DoesPieceFit(int Tetromino, int Rototation, int PosX, int PosY) {
  for (int px = 0; px < 4; px++) {
    for (int py = 0; py < 4; py++) {
      if (tetrominos[Tetromino][Rototation % 4][py][px] != 0) {
        int fieldX = PosX + px;
        int fieldY = PosY + py;
        if (fieldX < 0 || fieldX >= 11 || fieldY < 0 || fieldY >= 20) {
          return false;
        }

        if (Field[fieldY][fieldX] != 0) {
          return false;
        }
      }
    }
  }
  return true;
};

int main() {
  char input;
  while (true) {
    usleep(50000);
    // timing
    input = getch();
    switch (input) {
    case 'a':
    case 'A':
      if (DoesPieceFit(CurrentPiece, CurrentRotation, CurrentX - 1, CurrentY))
        CurrentX--;
      break;
    case 's':
    case 'S':
      if (DoesPieceFit(CurrentPiece, CurrentRotation, CurrentX, CurrentY + 1))
        CurrentY++;
      break;
    case 'd':
    case 'D':
      if (DoesPieceFit(CurrentPiece, CurrentRotation, CurrentX + 1, CurrentY))
        CurrentX++;
      break;
    case 'q':
    case 'Q':
      if (DoesPieceFit(CurrentPiece, CurrentRotation + 3, CurrentX, CurrentY))
        CurrentRotation += 3;
      break;
    case 'e':
    case 'E':
      if (DoesPieceFit(CurrentPiece, CurrentRotation + 1, CurrentX, CurrentY))
        CurrentRotation++;
      break;
    }
    if (TimeCounter >= 20) {
      TimeCounter = 0;
      if (DoesPieceFit(CurrentPiece, CurrentRotation, CurrentX, CurrentY + 1)) {
        CurrentY++;
      } else {
        for (int px = 0; px < 4; px++) {
          for (int py = 0; py < 4; py++) {
            if (tetrominos[CurrentPiece][CurrentRotation % 4][py][px]) {
              Field[CurrentY + py][CurrentX + px] = 1;
            }
          }
        }
        if (DoesPieceFit(CurrentPiece, CurrentRotation, 6, 0)) {
          for (int py = 0; py < 4; py++) {
            if (CurrentY + py < 20) {
              bool LineFull = true;
              for (int px = 1; px < 12; px++) {
                LineFull &= Field[CurrentY + py][CurrentX + px] != 0;
              }
              if (LineFull) {
                Score = Score * 0.5 + 10;
                // Clear the current line
                for (int px = 1; px < 12; px++) {
                  Field[CurrentY][CurrentX + px] = 0;
                }
                // Shift rows down
                for (int py = CurrentY; py > 0; py--) {
                  for (int px = 0; px < 11; px++) {
                    Field[py][px] = Field[py - 1][px];
                  }
                }
              }
            }
          }
          CurrentX = 6;
          CurrentY = 0;
          CurrentPiece = rand() % 7;
          CurrentRotation = rand() % 4;
        } else {
          break;
        }
      }
    }
    TimeCounter++;
    for (int i = 0; i < 20; i++) {
      for (int j = 0; j < 11; j++) {
        screen[i][j] = Field[i][j];
      }
    }
    for (int px = 0; px < 4; px++) {
      for (int py = 0; py < 4; py++) {
        if (tetrominos[CurrentPiece][CurrentRotation % 4][py][px]) {
          screen[CurrentY + py][CurrentX + px] = 1;
        }
      }
    }
    // removeLines();
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
    printf("a s d to move left down rigt and q e for rototating ");
    fflush(stdout);
  };
  printf("\033[H\033[Jgame over your score is %i\n", Score);
  return 0;
}

#include <stdio.h>
#include <termios.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

short px = 3, py = 3;
unsigned short FoodX, FoodY;
char direction = 'a';
unsigned int size = 3;

unsigned short map[20][20] = {0};
unsigned short screen[20][20];

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

int main() {
    direction = 'a';
    printf("press anything to start also it is a bit buggy game and i am lazy to debug it and it is synchronous");
    int minus;
    FoodX = rand() % 20;
    FoodY = rand() % 20;
    while (true) {
        usleep(250000);
        char new_direction = getch();
        if (new_direction != -1) {
            direction = new_direction; // Only update direction if key is pressed
        }
        switch (direction) {
            case 'a': case 'A': py--; break;
            case 'w': case 'W': px--; break;
            case 's': case 'S': px++; break;
            case 'd': case 'D': py++; break;
        }
        px = (px+20) % 20;
        py = (py+20) % 20;
        if (map[px][py] != 0) {
            break;
        }
        minus = 1;
        if ((px == FoodX) && (py == FoodY)) {
            do {
                FoodX = rand() % 20;
                FoodY = rand() % 20;
            } while (map[FoodX][FoodY] != 0);
            size++;
            minus = 0;
        }
        map[px][py] = size;

        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 20; j++) {
                screen[i][j] = map[i][j];
            }
        }
        screen[FoodX][FoodY] = 32767;


        printf("\033[H\033[J");
        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 20; j++) {
                switch(screen[i][j]) {
                case 0: printf(".");
                break;
                case 32767: printf("@");
                break;
                default: 
                    printf("\u25A0"); map[i][j] = map[i][j] - minus;
                    break;
                }
        }
    printf("\n");
    }
    printf("Player Position: (%i, %i)\n", px, py);
    }
    printf("\033[H\033[J");
    printf("Your score is %i\n", (size-3)*100);
    return 0;
}
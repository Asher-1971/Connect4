#include <stdio.h>
#include <unistd.h>

void wait_for_x(void) {
    int c;
    while ((c = getchar()) != EOF) {
        if (c == 'x' || c == 'X') {
            while ((c = getchar()) != '\n' && c != EOF) { }
            break;
        }
    }
}

int main(void) {
    // Open alternate screen buffer (blocks scrolling up to see history)
    printf("\033[?1049h\033[3J\033[2J\033[H");

    for (int i = 0; i < 5; i++) {
        printf("2D array printed, press 'x' and Enter to continue...");
        fflush(stdout);

        wait_for_x();

        // Clear visible screen + scrollback buffer, reset cursor
        printf("\033[3J\033[2J\033[H");
        fflush(stdout);

        sleep(1);
        printf("One two three\n");
        printf("%d\n", i);
    }

   
    printf("\033[?1049l");
    return 0;
}
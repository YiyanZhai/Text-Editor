#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/********************* data *********************/

// Original terminal attributes
struct termios orig_termios;


/******************* terminal *******************/

/** Error handling
 * die prints an error message and exits the program.
 **/
void die(const char *s) {
  perror(s);
  exit(1);
}

/** To DISABLE raw mode at exit **/
void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

/**
 * Modify, and set terminal’s attributes
 **/
void enableRawMode() {
    // READ the current attributes into a struct with tcgetattr()
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");

    // Restore terminal’s original attributes when program exits
    atexit(disableRawMode);
    struct termios raw = orig_termios;

    /** MODIFY FLAGS **/
    // By bitwise-ANDing the NOTed-value with each flag field (to force the corresponding bit to be 0).
    
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // input flags:
    // IXON - turn it off to disable Ctrl-S and Ctrl-Q
    // ICRNL - turn it off to fix Ctrl-L
    // BRKINT - a break condition will cause a SIGINT to be sent
    // INPCK - enables parity checking
    // ISTRIP - causes the 8th bit of each input byte to be stripped (set to 0)
    
    raw.c_oflag &= ~(OPOST);
    // output flags:
    // OPOST - output processing
    
    raw.c_cflag |= (CS8);
    // control flags
    // CS8 (not a flag) - set the character size (CS) to 8 bits per byte.
    
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    // local flags - “dumping ground for other state”:
    // ECHO - make each typed key printed to the terminal
    // It is useful in canonical mode, but not when carefully render a user interface in raw mode.
    // ICANON - canonical mode. To read input byte-by-byte, instead of line-by-line.
    // ISIG - Ctrl-C and Ctrl-Z signals. Now C is 3, Z is 26.
    // IEXTEN - Ctrl-V

    // A timeout for read()
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    // Pass to tcsetattr() the modified struct to WRITE the new attributes
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
    // TCSAFLUSH: specifies when to apply the change
    // Wait for all pending output to be written to the terminal, discard any input that hasn’t been read.
}


/********************* init *********************/

int main(){
    enableRawMode();
    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        
        // iscntrl() tests control (nonprintable) character.
        if (iscntrl(c)) {
        printf("%d\r\n", c);
        } else {
        printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;
    }
    return 0;
}


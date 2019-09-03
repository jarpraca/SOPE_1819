
#include <unistd.h>
#include <string.h>
#include <termios.h>

#define MAX_LENGTH 15

int main(void)
{
    struct termios term, oldterm;
    int i=0;
    char pass[MAX_LENGTH+1], ch, echo = '*';
    write(STDOUT_FILENO, "\nPassword? ", 11);
    tcgetattr(STDIN_FILENO, &oldterm);
    term = oldterm;
    term.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
    while (i < MAX_LENGTH && read(STDIN_FILENO, &ch, 1) &&
    ch != '\n') {
    pass[i++] = ch;
    write(STDOUT_FILENO, &echo, 1);
    }
    pass[i] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldterm);

    write(STDOUT_FILENO, "\n\nPassword: ", 12);
    write(STDOUT_FILENO, pass, strlen(pass));
    write(STDOUT_FILENO, "\n", 1);
    return 0;
} 
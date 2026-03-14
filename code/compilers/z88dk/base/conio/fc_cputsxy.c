#include <conio.h>
#include "fujinet-compiler.h"

void fc_cputsxy(unsigned char x, unsigned char y, const char *message) {
    gotoxy(x,y);
    cputs(message);
}
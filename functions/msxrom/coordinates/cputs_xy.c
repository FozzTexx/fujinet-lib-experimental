#include <conio.h>
#include "fujinet-functions.h"

void cputs_xy(unsigned char x, unsigned char y, const char * message) {
    gotoxy(x,y);
    cputs(s);
}
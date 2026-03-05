#include <conio.h>
#include "fujinet-functions.h"

void cputs_xy(unsigned int x, unsigned int y, const char *message) {
    gotoxy(x,y);
    cputs(message);
}
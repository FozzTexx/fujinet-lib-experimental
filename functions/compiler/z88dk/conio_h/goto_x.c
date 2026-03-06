#include <conio.h>
#include "fujinet-functions-compiler.h"

void goto_x(unsigned char x)
{
    unsigned char y;

    y = wherey();

    gotoxy(x, y);
}
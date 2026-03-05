#include <conio.h>
#include "fujinet-functions-compiler.h"

void goto_x(unsigned int x)
{
    unsigned int y;

    y = wherey();

    gotoxy(x, y);
}
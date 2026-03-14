#include <conio.h>
#include "fujinet-compiler.h"

void fc_gotox(unsigned char x)
{
    unsigned char y;

    y = wherey();

    gotoxy(x, y);
}
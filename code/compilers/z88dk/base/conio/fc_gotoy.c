#include <conio.h>
#include "fujinet-compiler.h"

void fc_gotox(unsigned char y)
{
    unsigned char x;

    x = wherey();

    gotoxy(x, y);
}
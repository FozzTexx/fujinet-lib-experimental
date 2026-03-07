/* 
extern void __FASTCALL__ cputs_xy(unsigned char x, unsigned char y, const char* s);
extern void __FASTCALL__ goto_x(unsigned char x); 
*/

/* 
 * It's not a good idea to overwrite nor override cputsxy() and gotox(),
 *  since they are build-in functions of CC65, so instead I made my own
 *  that can be used by both CC65, Z88DK, and any other compilers that will be added.
 */
void fc_cputsxy(unsigned char x, unsigned char y, const char* s);
void fc_gotox(unsigned char x); 
void fc_gotoy(unsigned char y);
void fc_gotoxy(unsigned char x, unsigned char y);
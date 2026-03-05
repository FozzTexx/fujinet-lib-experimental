/* 
extern void __FASTCALL__ cputs_xy(unsigned char x, unsigned char y, const char* s);
extern void __FASTCALL__ goto_x(unsigned char x); 
*/

/* It's not a good idea to overwrite nor override cputsxy() and gotox(),
 *  since they are build-in functions of CC65, so instead I made my own
 *  that can be used by both CC65 and Z88DK
 */
void cputs_xy(unsigned char x, unsigned char y, const char* s);
void goto_x(unsigned char x); 

bool fuji_base64_encode_compute(void);
bool fuji_base64_encode_input(char *s, uint16_t len);
bool fuji_base64_encode_length(unsigned long *len);
bool fuji_base64_encode_output(char *s, uint16_t len);

bool fuji_base64_decode_compute(void);
bool fuji_base64_decode_input(char *s, uint16_t len);
bool fuji_base64_decode_length(unsigned long *len);
bool fuji_base64_decode_output(char *s, uint16_t len);

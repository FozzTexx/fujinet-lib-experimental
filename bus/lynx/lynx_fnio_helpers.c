/**
 *  for Atari Lynx
 */

#include <lynx.h>
#include <serial.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "lynxfnio.h"
#include "fujinet-commands.h"



/**
 * @brief calculates the checksum on the packet and stores in global var
 * @param *b pointer to buffer
 * @param len length of buffer
 */
void _checksum(char *b, unsigned short len)
{
  register unsigned short i;


  // initialize checksum to zero
  _ck = 0;

  for (i = 0; i < len; ++i)
    _ck ^= b[i];

  return;
}


/**
 * @brief loop to get data that is used many times
 * @param b The buffer to receive into
 */
unsigned char _serial_get_loop(void)
{
  clock_t start, now;

  start = clock();
  while (ser_get(&_r) == SER_ERR_NO_DATA) {
    now = clock();
    if (((now - start) / CLOCKS_PER_SEC) > LYNX_TIMEOUT) {
      _fn_error = FNIO_ERR_TIMEOUT;
      return(0);
    }
  }

  return(1);
}


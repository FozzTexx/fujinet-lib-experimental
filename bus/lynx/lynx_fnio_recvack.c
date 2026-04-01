/**
 *  for Atari Lynx
 */

#include <lynx.h>
#include <serial.h>
#include <stdlib.h>
#include <stdio.h>
#include "lynxfnio.h"
#include "fujinet-commands.h"



void fnio_flush_recv(void)
{
  while (ser_get(&_r) != SER_ERR_NO_DATA);
}


// receive an ACK or NAK
// returns 1 on ACK received or 0 on NAK
// Call after commands that don't send back any data
unsigned char fnio_recv_ack(void)
{
	unsigned char t;

  // reset error status
  _fn_error = FNIO_ERR_NONE;

  t = _serial_get_loop();
  if (!t)
  	return(0);

  if (_r == FUJICMD_ACK)
    return(1);
  else {
	  _fn_error = FNIO_ERR_CMD_FAILED;
    fnio_flush_recv();
    return(0);
	}
}

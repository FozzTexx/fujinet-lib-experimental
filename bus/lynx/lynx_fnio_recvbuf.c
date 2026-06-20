/**
 *  for Atari Lynx
*/

#include <lynx.h>
#include <serial.h>
#include <stdlib.h>
#include <stdio.h>
#include "lynxfnio.h"
#include "fujinet-commands.h"



unsigned char fnio_recv_buf(char *buf, unsigned int *len)
{
  register unsigned short i;
  unsigned char t;


  // reset error status
  _fn_error = FNIO_ERR_NONE;

  // Get first length byte
  t = _serial_get_loop();
  if (!t)
	return(0);
  *len = _r << 8;

  // Get second length byte
  t = _serial_get_loop();
  if (!t)
  	return(0);
  *len |= _r & 0xFF;

  if (*len > FNIO_TX_LEN_MAX)       // no more than LEN_MAX bytes
	  *len = FNIO_TX_LEN_MAX;

  // Now get the payload
  for (i=0; i<*len; ++i) {
	  t = _serial_get_loop();
	  if (!t)
	  	return(0);
   	buf[i] = _r;
  }

  // Get the checksum
  t = _serial_get_loop();
  if (!t)
  	return(0);                            // timeout

  // checksum matches?
  _checksum(buf, *len);
  if (_r == _ck) {
	  ser_put(FUJICMD_ACK);								  // ACK
    ser_get((char *) &_r);                // get reflected data
    return(1);                            // succes, checksum matches
  }
  else {
	  _fn_error = FNIO_ERR_RECV_CHK;
	  ser_put(FUJICMD_NAK); 								// NACK, checksum bad
    ser_get((char *) &_r);                // get reflected data
	  *len = 0;											        // return zero length
    return(0);                            // checksum bad
  }
}

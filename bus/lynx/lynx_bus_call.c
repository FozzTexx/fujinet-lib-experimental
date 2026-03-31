#include "fujinet-fuji.h"
#include "lynxfnio.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>


char _lynx_packet[1024];
uint16_t _lynx_len;


// fields, aux3, aux4 only used with other systems, not
// Atari. Commented out to prevent warning about unused arguments.
bool fuji_bus_call(uint8_t device, uint8_t fuji_cmd, uint8_t fields,
		   uint8_t aux1, uint8_t aux2, uint8_t aux3, uint8_t aux4,
		   const void *data, size_t data_length,
		   void *reply, size_t reply_length)
{
  uint8_t r;
  uint8_t num_bytes;


  // Reset our data length counter
  _lynx_len = 0;

  // Command is first
  _lynx_packet[_lynx_len] = fuji_cmd;
  _lynx_len++;

  // Build the packet from aux values
  numbytes = fuji_field_numbytes(fields);
  if (numbytes) {
    _lynx_packet[_lynx_len++] = aux1;
    numbytes--;
  }
  if (numbytes) {
    _lynx_packet[_lynx_len++] = aux2;
    numbytes--;
  }
  if (numbytes) {
    _lynx_packet[_lynx_len++] = aux3;
    numbytes--;
  }
  if (numbytes) {
    _lynx_packet[_lynx_len++] = aux4;
    numbytes--;
  }

  // Add data if it exists
  if (data) {
    memcpy(&_lynx_packet[_lynx_len], data, data_len);
    lynx_len += data_len;
  }

  // Send the command (and data)
  r = fnio_send_buf(device, &_lynx_packet[0], _lynx_len);
  if (!r)
    return(false);
  
  // Get reply if one expected
  if (reply) {
    r = fnio_recv_buf(reply, &_lynx_len);
    if ((!r) || (_lynx_len == 0))
      return(false);
  }
  // Get ACK from Fujinet that command succeeded
  else {
    r = fnio_recv_ack();
    if (!r)
      return(false);
  }

  return(true);
}

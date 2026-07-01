#include "fuji_bus_appkey.h"

#include <fujinet-commands.h>
#include <fujinet-bus.h>

FNAppKeyString appkey_buf;

/*
  appkeys are variable length strings. CoCo drivewire is serial but
  drivewireFuji in the FujiNet firmware seems to be using fixed length
  packets with a header to indicate the true length of the key, same
  as Atari SIO. On write the aux1/aux2 fields are combined into a
  uint16_t. On read, there is an extra 2 bytes of header to indicate
  how much of the fixed block represents the string.
*/

bool fuji_bus_appkey_read(void *string, uint16_t *length)
{
  // Caller may not have room for length header so use our own buffer to read
  if (!FUJICALL_RV(FUJICMD_READ_APPKEY, &appkey_buf, sizeof(appkey_buf)))
    return false;
  *length = appkey_buf.length;
  memmove(string, appkey_buf.data, appkey_buf.length);
  return true;
}


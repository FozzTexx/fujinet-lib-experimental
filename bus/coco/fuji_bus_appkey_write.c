#include "fuji_bus_appkey.h"

#include <fujinet-commands.h>
#include <fujinet-bus.h>

/*
  appkeys are variable length strings. CoCo drivewire is serial but
  drivewireFuji in the FujiNet firmware seems to be using fixed length
  packets with a header to indicate the true length of the key, same
  as Atari SIO. On write the aux1/aux2 fields are combined into a
  uint16_t. On read, there is an extra 2 bytes of header to indicate
  how much of the fixed block represents the string.
*/

bool fuji_bus_appkey_write(void *string, uint16_t length)
{
  // The drivewire firmware decodes WRITE_APPKEY's length MSB-first
  // (`lenh<<8 | lenl`), matching the stable fujinet-lib's struct-copy
  // wire format. FUJICALL_B12_D would send LSB-first, so use
  // FUJICALL_A1_A2_D with the bytes in MSB,LSB order instead.
  return FUJICALL_A1_A2_D(FUJICMD_WRITE_APPKEY,
                          U16_MSB(length), U16_LSB(length),
                          string, MAX_APPKEY_LEN);
}

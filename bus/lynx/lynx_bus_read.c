#include "fujinet-fuji.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>


// fields, aux3, aux4 only used with other systems, not
// Atari. Commented out to prevent warning about unused arguments.
bool fuji_bus_call(uint8_t device, uint8_t fuji_cmd, uint8_t /*fields*/,
		   uint8_t aux1, uint8_t aux2, uint8_t /*aux3*/, uint8_t /*aux4*/,
		   const void *data, size_t data_length,
		   void *reply, size_t reply_length)
{


}

uint16_t fuji_bus_read(uint8_t device, void *buffer, size_t length)
{
  NETCALL_B12_RV(FUJICMD_READ, device - FUJI_DEVICEID_NETWORK + 1, length, buffer, length);
  return length;
}

uint16_t fuji_bus_write(uint8_t device, const void *buffer, size_t length)
{
  NETCALL_B12_D(FUJICMD_WRITE, device - FUJI_DEVICEID_NETWORK + 1, length, buffer, length);
  return length;
}

#include <fujinet-bus.h>
#include <fujinet-commands.h>

/* ComLynx WRITE takes no length params: everything after the command
   byte in the packet is payload. */
size_t network_bus_write(uint8_t device, const void *buffer, size_t length)
{
  if (!NETCALL_D(FUJICMD_WRITE, device - FUJI_DEVICEID_NETWORK + 1, buffer, length))
    return 0;
  return length;
}

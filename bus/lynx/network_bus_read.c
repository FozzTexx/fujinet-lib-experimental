#include <fujinet-bus.h>
#include <fujinet-commands.h>

#include "lynxfnio.h"

/* ComLynx READ ignores the requested length: the FujiNet sends
   min(bytes waiting, SERIAL_PACKET_SIZE) and fnio_recv_buf() fails if
   that exceeds the caller's buffer. Return the length actually
   received, which may be less than requested. */
size_t network_bus_read(uint8_t device, void *buffer, size_t length)
{
  if (!NETCALL_RV(FUJICMD_READ, device - FUJI_DEVICEID_NETWORK + 1, buffer, length))
    return 0;
  return _lynx_len;
}

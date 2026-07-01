#include "fuji_net_call.h"
#include "dw.h"

#include <fujinet-int.h>
#include <fujinet-bus.h>
#include <fujinet-commands.h>

uint16_t fuji_bus_write(uint8_t device, const void *buffer, size_t length)
{
  nw_header.opcode = OP_NET;
  nw_header.unit = device - FUJI_DEVICEID_NETWORK + 1;
  nw_header.cmd = FUJICMD_WRITE;

  bus_ready();
  dwwrite((uint8_t *) &nw_header, sizeof(nw_header));
  dwwrite((uint8_t *) &length, sizeof(length));
  dwwrite((uint8_t *) buffer, length);

  if (network_get_error(nw_header.unit))
    length = 0;
  return length;
}


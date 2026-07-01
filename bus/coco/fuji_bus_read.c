#include "fuji_net_call.h"
#include "dw.h"

#include <fujinet-int.h>
#include <fujinet-bus.h>
#include <fujinet-commands.h>

uint16_t fuji_bus_read(uint8_t device, void *buffer, size_t length)
{
  nw_header.opcode = OP_NET;
  nw_header.unit = device - FUJI_DEVICEID_NETWORK + 1;
  nw_header.cmd = FUJICMD_READ;

  bus_ready();
  dwwrite((uint8_t *) &nw_header, sizeof(nw_header));
  dwwrite((uint8_t *) &length, sizeof(length));
  network_get_response(nw_header.unit, (uint8_t *) buffer, length);

  return length;
}


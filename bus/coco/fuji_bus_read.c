#include "fb_header.h"
#include "fujinet-fuji-coco.h"
#include "fujinet-network-coco.h"
#include "dw.h"

#include <fujinet-bus.h>
#include <fujinet-commands.h>

uint16_t fuji_bus_read(uint8_t device, void *buffer, size_t length)
{
  fb_header.opcode = OP_NET;
  fb_header.fn.net.unit = device - FUJI_DEVICEID_NETWORK + 1;
  fb_header.fn.net.cmd = FUJICMD_READ;

  bus_ready();
  dwwrite((uint8_t *) &fb_header, sizeof(fb_header));
  dwwrite((uint8_t *) &length, sizeof(length));
  network_get_response(fb_header.fn.net.unit, (uint8_t *) buffer, length);

  return length;
}


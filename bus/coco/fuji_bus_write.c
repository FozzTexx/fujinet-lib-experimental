#include "fb_header.h"
#include "fujinet-fuji-coco.h"
#include "fujinet-network-coco.h"
#include "dw.h"

#include <fujinet-bus.h>
#include <fujinet-commands.h>

uint16_t fuji_bus_write(uint8_t device, const void *buffer, size_t length)
{
  fb_header.opcode = OP_NET;
  fb_header.fn.net.unit = device - FUJI_DEVICEID_NETWORK + 1;
  fb_header.fn.net.cmd = FUJICMD_WRITE;

  bus_ready();
  dwwrite((uint8_t *) &fb_header, sizeof(fb_header));
  dwwrite((uint8_t *) &length, sizeof(length));
  dwwrite((uint8_t *) buffer, length);

  if (network_get_error(fb_header.fn.net.unit))
    length = 0;
  return length;
}


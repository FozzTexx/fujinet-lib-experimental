#include "dw.h"
#include "fuji_net_call.h"
#include "pack_payload.h"
#include "fujinet-fuji-coco.h"

#include <fujinet-int.h>
#include <fujinet-bus.h>

typedef struct {
  uint8_t opcode;
  uint8_t cmd;
} fujibus_header;

static fujibus_header fb_header;
uint8_t *fb_packet = NULL;

bool fuji_bus_call(uint8_t device, uint8_t fuji_cmd, uint8_t fields,
		   uint8_t aux1, uint8_t aux2, uint8_t aux3, uint8_t aux4,
		   const void *data, size_t data_length,
		   void *reply, size_t reply_length)
{
  uint16_t idx;


  if (device >= FUJI_DEVICEID_NETWORK  && device <= FUJI_DEVICEID_NETWORK_LAST)
    return fuji_net_call(device - FUJI_DEVICEID_NETWORK + 1, fuji_cmd, fields,
                         aux1, aux2, aux3, aux4, data, data_length, reply, reply_length);

  if (device != FUJI_DEVICEID_CLOCK && device != FUJI_DEVICEID_FUJINET)
    return false;

  // Use sbrk(0) to get pointer to unused memory at top of program. No
  // need to move the sbrk() since we only need this space temporarily
  // and nothing else will call sbrk() to try to use this space. Since
  // we don't allocate it we also don't need to free it. Probably
  // unsafe.
  fb_packet = (uint8_t *) sbrk(0);

  if (device == FUJI_DEVICEID_CLOCK)
    fb_header.opcode = OP_CLOCK;
  else
    fb_header.opcode = OP_FUJI;

  fb_header.cmd = fuji_cmd;

  idx = pack_payload(fb_packet, fields, aux1, aux2, aux3, aux4, data, data_length);

  bus_ready();
  dwwrite((unsigned char *) &fb_header, sizeof(fb_header));
  if (idx)
    dwwrite(fb_packet, idx);

  if (device == FUJI_DEVICEID_CLOCK)
  {
    if (reply)
      return dwread((byte *)reply, reply_length) ? true : false;
  }
  else
  {
    if (fuji_get_error())
      return false;
    if (reply)
      return (bool)fuji_get_response((unsigned char *)reply, reply_length);
  }
  return true;
}

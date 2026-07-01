#include "fuji_net_call.h"
#include "dw.h"
#include "fujinet-network-coco.h"

bool fuji_net_call(uint8_t unit, uint8_t fuji_cmd, uint8_t fields,
		   uint8_t aux1, uint8_t aux2, uint8_t aux3, uint8_t aux4,
		   const void *data, size_t data_length,
		   void *reply, size_t reply_length)
{
  uint8_t err;
  uint16_t idx;


  // Use sbrk(0) to get pointer to unused memory at top of program. No
  // need to move the sbrk() since we only need this space temporarily
  // and nothing else will call sbrk() to try to use this space. Since
  // we don't allocate it we also don't need to free it. Probably
  // unsafe.
  fb_packet = (uint8_t *) sbrk(0);

  nw_header.opcode = OP_NET;
  nw_header.unit = unit;
  nw_header.cmd = fuji_cmd;

  idx = pack_payload(fb_packet, fields, aux1, aux2, aux3, aux4, data, data_length);

  bus_ready();
  dwwrite((unsigned char *) &nw_header, sizeof(nw_header));
  if (idx)
    dwwrite(fb_packet, idx);

  err = network_get_error(unit);
  if (err)
    return false;

  if (reply) {
    err = network_get_response(unit, (unsigned char *) reply, reply_length);
    return !err;
  }

  return true;
}


#include "fujinet-bus-msx.h"
#include "fujinet-commands.h"
#include "fujinet-unapi-msx.h"

#undef HEXDUMP
#if defined(DEBUG) || defined(HEXDUMP)
#define COLUMNS 16

static void hexdump(uint8_t *buffer, int count)
{
  int outer, inner;
  uint8_t c;


  for (outer = 0; outer < count; outer += COLUMNS) {
    for (inner = 0; inner < COLUMNS; inner++) {
      if (inner + outer < count) {
	c = buffer[inner + outer];
	printf("%02x ", c);
      }
      else
	printf("   ");
    }
    printf(" |");
    for (inner = 0; inner < COLUMNS && inner + outer < count; inner++) {
      c = buffer[inner + outer];
      if (c >= ' ' && c <= 0x7f)
	printf("%c", c);
      else
	printf(".");
    }
    printf("|\n");
  }

  return;
}
#endif /* HEXDUMP */

/* Static rather than automatic on purpose: the parameter block must not be in
   page 1 or page 2, and on MSX the stack usually is not - but BSS, which this
   lands in, reliably is not. */
static FujiNetParams params;

bool fuji_bus_call(uint8_t device, uint8_t fuji_cmd, uint8_t fields,
		   uint8_t aux1, uint8_t aux2, uint8_t aux3, uint8_t aux4,
		   const void *data, size_t data_length,
		   void *reply, size_t reply_length)
{
  params.device = device;
  params.command = fuji_cmd;
  params.aux_descr = fields;

  params.aux1 = aux1;
  params.aux2 = aux2;
  params.aux3 = aux3;
  params.aux4 = aux4;

  if (reply) {
    params.buffer = reply;
    params.length = reply_length;
#ifdef DEBUG
    printf("FUJINET READ %d\n", params.length);
    hexdump(&params, sizeof(params));
#endif
    return fuji_unapi_call(FUJI_CALL_READ, &params);
  }

  params.buffer = (void *) data;
  params.length = data_length;
#ifdef DEBUG
  printf("FUJINET WRITE %d\n", params.length);
#endif
  return fuji_unapi_call(FUJI_CALL_WRITE, &params);
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

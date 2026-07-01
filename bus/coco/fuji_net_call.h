#include <fujinet-int.h>

typedef struct {
  byte opcode;
  byte unit;
  byte cmd;
} fujinw_header;

extern fujinw_header nw_header;
extern uint8_t *fb_packet;

extern bool fuji_net_call(uint8_t unit, uint8_t fuji_cmd, uint8_t fields,
                          uint8_t aux1, uint8_t aux2, uint8_t aux3, uint8_t aux4,
                          const void *data, size_t data_length,
                          void *reply, size_t reply_length);

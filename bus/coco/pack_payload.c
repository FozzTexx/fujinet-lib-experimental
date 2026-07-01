#include "pack_payload.h"

uint16_t pack_payload(uint8_t *buf, uint8_t fields,
                      uint8_t aux1, uint8_t aux2, uint8_t aux3, uint8_t aux4,
                      const void *data, size_t data_length)
{
  uint16_t idx = 0;
  uint16_t numbytes = fuji_field_numbytes(fields);
  if (numbytes) { buf[idx++] = aux1; numbytes--; }
  if (numbytes) { buf[idx++] = aux2; numbytes--; }
  if (numbytes) { buf[idx++] = aux3; numbytes--; }
  if (numbytes) { buf[idx++] = aux4; numbytes--; }
  if (data) {
    memcpy(&buf[idx], data, data_length);
    idx += data_length;
  }
  return idx;
}


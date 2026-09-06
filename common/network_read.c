#include <fujinet-network.h>

uint16_t fn_bytes_read;

int16_t network_read(const char *devicespec, void *buf, uint16_t len)
{
  int16_t total;
  uint8_t *ubuf = (uint8_t *) buf;
  int16_t rlen = 0;


  for (total = 0; len; total += rlen, len -= rlen) {
    rlen = network_read_nb(devicespec, &ubuf[total], len);
    if (rlen < 1)
      break;
  }

  fn_bytes_read = total;
  if (!total && rlen < 0)
    return rlen;
  return total;
}


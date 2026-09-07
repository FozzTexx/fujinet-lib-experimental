#include <fujinet-network.h>

FN_ERR network_seek(const char *devicespec, uint32_t pos)
{
#ifdef BUILD_ATARI
  uint8_t buf[3];
#endif /* BUILD_ATARI */
  uint8_t nw_unit = network_unit(devicespec);


#ifdef BUILD_ATARI
  /* SIO takes the offset as a 3 byte little-endian data frame */
  buf[0] = pos & 0xFF;
  buf[1] = (pos >> 8) & 0xFF;
  buf[2] = (pos >> 16) & 0xFF;
  if (!NETCALL_D(FUJICMD_SEEK, nw_unit, buf, sizeof(buf)))
    return FN_ERR_IO_ERROR;
#else /* ! BUILD_ATARI */
  /* RS232 and unified buses take a 32-bit position parameter */
  if (!NETCALL_C1234(FUJICMD_SEEK, nw_unit, pos))
    return FN_ERR_IO_ERROR;
#endif /* BUILD_ATARI */
  return FN_ERR_OK;
}

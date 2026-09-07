#include <fujinet-network.h>

FN_ERR network_tell(const char *devicespec, uint32_t *pos)
{
  uint8_t buf[4];
  uint8_t nw_unit = network_unit(devicespec);


  buf[3] = 0;
#ifdef BUILD_ATARI
  /* SIO replies with a 3 byte little-endian position */
  if (!NETCALL_RV(FUJICMD_TELL, nw_unit, buf, 3))
    return FN_ERR_IO_ERROR;
#else /* ! BUILD_ATARI */
  /* RS232 and unified buses reply with 4 bytes little-endian */
  if (!NETCALL_RV(FUJICMD_TELL, nw_unit, buf, 4))
    return FN_ERR_IO_ERROR;
#endif /* BUILD_ATARI */
  *pos = ((uint32_t) buf[3] << 24) | ((uint32_t) buf[2] << 16)
    | ((uint16_t) buf[1] << 8) | buf[0];
  return FN_ERR_OK;
}

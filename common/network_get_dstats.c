#include <fujinet-network.h>

FN_ERR network_get_dstats(const char *devicespec, uint8_t cmd, uint8_t *dstats)
{
  uint8_t nw_unit = network_unit(devicespec);


  if (!NETCALL_A1_RV(FUJICMD_GET_DSTATS_VALUE, nw_unit, cmd, dstats, 1))
    return FN_ERR_IO_ERROR;
  return FN_ERR_OK;
}

#include <fujinet-network.h>

FN_ERR network_sgml_parse(const char *devicespec)
{
  uint8_t nw_unit = network_unit(devicespec);
  FN_ERR err = network_set_channel_mode(devicespec, NETWORK_CHANMODE_SGML);


  if (err != FN_ERR_OK)
    return err;

  if (!NETCALL(FUJICMD_PARSE, nw_unit))
    return FN_ERR_IO_ERROR;
  return FN_ERR_OK;
}

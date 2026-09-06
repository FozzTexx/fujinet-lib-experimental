#include <fujinet-network.h>

FN_ERR network_udp_set_destination(const char *dest_spec)
{
  uint8_t nw_unit = network_unit(dest_spec);


  if (!NETCALL_A1_A2_D(FUJICMD_SET_DESTINATION, nw_unit, 0, 0, dest_spec,
                       NETWORK_FS_LEN(dest_spec)))
    return FN_ERR_IO_ERROR;
  return FN_ERR_OK;
}

#include <fujinet-network.h>

FN_ERR network_set_timer_rate(const char *devicespec, uint8_t rate)
{
  uint8_t nw_unit = network_unit(devicespec);


  // Rate in ms goes in both aux bytes: SIO reads aux1, RS232 reads aux2
  if (!NETCALL_A1_A2(FUJICMD_SET_TIMER_RATE, nw_unit, rate, rate))
    return FN_ERR_IO_ERROR;
  return FN_ERR_OK;
}

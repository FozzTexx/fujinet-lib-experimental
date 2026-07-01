#include "clock.h"

#include <fujinet-clock.h>

uint8_t clock_get_time(uint8_t *time_data, TimeFormat format)
{
  if ((uint8_t) format >= TIMEFORMAT_COUNT)
    return FN_ERR_BAD_CMD;
  return CLKCALL_A1_RV(clk_cmd[format], 0, time_data, clk_reply_len[format])
         ? FN_ERR_OK : FN_ERR_IO_ERROR;
}

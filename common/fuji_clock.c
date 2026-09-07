#include <fujinet-clock.h>

#ifdef _CMOC_VERSION_
#include <cmoc.h>
#else
#include <string.h>
#endif

const uint8_t clk_cmd[TIMEFORMAT_COUNT] = {
  CLK_CMD_SIMPLE_BINARY,
  CLK_CMD_PRODOS_BINARY,
  CLK_CMD_APETIME_BINARY,
  CLK_CMD_TZ_ISO_STRING,
  CLK_CMD_UTC_ISO_STRING,
  CLK_CMD_APPLE3_SOS_BINARY,
  CLK_CMD_SIMPLE_BINARY_WITH_HUNDREDTHS
};

const uint8_t clk_reply_len[TIMEFORMAT_COUNT] = { 7, 4, 6, 25, 25, 19, 8 };

static bool platform_clk_set_tz_call(uint8_t cmd, const char *tz)
{
  size_t len = strlen(tz) + 1;
  return CLKCALL_B12_D(cmd, len, tz, len);
}

static bool platform_clk_get_tz_len(uint8_t *len_out)
{
  return CLKCALL_RV(APETIMECMD_GETTZ_LEN, len_out, 1);
}

#include "fuji_clock_common.h"

/* ---- public API ---- */

uint8_t clock_set_tz_default(const char *tz)
{
  return clock_set_tz_common(tz);
}

uint8_t clock_get_tz_default(char *tz)
{
  return clock_get_tz_common(tz);
}

uint8_t clock_get_time_default(uint8_t *time_data, TimeFormat format)
{
  return clock_get_time_common(time_data, format, false);
}

uint8_t clock_get_time_tz_default(uint8_t *time_data, const char *tz, TimeFormat format)
{
  return clock_get_time_tz_common(time_data, tz, format);
}

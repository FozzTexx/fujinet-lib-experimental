// PLATFORM_CLK_TIME_CALL must be defined before including fuji_clock_common.h
#define PLATFORM_CLK_TIME_CALL(format, alt, buf, len) \
  CLKCALL_A1_RV(clk_cmd[format], (alt) ? 1 : 0, buf, len)
#include "fuji_clock_common.h"

#define PLATFORM_TZCMD_ALT   APETIMECMD_SETTZ
#define PLATFORM_TZCMD_MAIN  APETIMECMD_SETTZ_ALT

static const uint8_t _clk_cmd[TIMEFORMAT_COUNT] = {
  CLK_CMD_SIMPLE_BINARY,
  CLK_CMD_PRODOS_BINARY,
  CLK_CMD_APETIME_BINARY,
  CLK_CMD_TZ_ISO_STRING,
  CLK_CMD_UTC_ISO_STRING,
  CLK_CMD_APPLE3_SOS_BINARY,
  CLK_CMD_SIMPLE_BINARY_WITH_HUNDREDTHS
};
const uint8_t *clk_cmd = _clk_cmd;
const uint8_t clk_reply_len[TIMEFORMAT_COUNT] = { 7, 4, 6, 25, 25, 19, 8 };

static uint8_t clock_set_alternate_tz_default(const char *tz)
{
  return clock_set_alternate_tz_common(tz);
}

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

static bool platform_clk_set_tz_call(uint8_t cmd, const char *tz)
{
  size_t len = strlen(tz) + 1;
  return CLKCALL_B12_D(cmd, len, tz, len);
}

static bool platform_clk_get_tz_len(uint8_t *len_out)
{
  return CLKCALL_RV(APETIMECMD_GETTZ_LEN, len_out, 1);
}

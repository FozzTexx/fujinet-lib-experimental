/* fuji_clock_common.h — shared logic included by each platform's fuji_clock*.c
 *
 * Each platform .c file must, before including this header, define:
 *   PLATFORM_CLK_TIME_CALL(format, alt, buf, len)   -> bool
 *   PLATFORM_TZCMD_MAIN / PLATFORM_TZCMD_ALT        -> uint8_t command codes
 * and implement:
 *   static bool platform_clk_set_tz_call(uint8_t cmd, const char *tz);
 *   static bool platform_clk_get_tz_len(uint8_t *len_out);
 */

#ifndef FUJI_CLOCK_COMMON_H
#define FUJI_CLOCK_COMMON_H

#include <fujinet-clock.h>

static uint8_t clk_result(bool ok)
{
  return ok ? FN_ERR_OK : FN_ERR_IO_ERROR;
}

static uint8_t clock_get_time_common(uint8_t *time_data, TimeFormat format, bool alt)
{
  bool success;

  if ((uint8_t) format >= TIMEFORMAT_COUNT)
    return FN_ERR_BAD_CMD;

  success = PLATFORM_CLK_TIME_CALL(format, alt, time_data, clk_reply_len[format]);
  if (success)
    time_data[clk_reply_len[format]] = 0;
  return clk_result(success);
}

static uint8_t clock_get_time_tz_common(uint8_t *time_data, const char *tz, TimeFormat format)
{
  uint8_t rc;

  if ((uint8_t) format >= TIMEFORMAT_COUNT)
    return FN_ERR_BAD_CMD;
  rc = clock_set_alternate_tz(tz);
  if (rc != FN_ERR_OK)
    return rc;
  return clock_get_time_common(time_data, format, /*alt=*/true);
}

static uint8_t clk_set_tz_raw(uint8_t cmd, const char *tz)
{
  return clk_result(platform_clk_set_tz_call(cmd, tz));
}

#ifdef UNUSED
static uint8_t clock_set_alternate_tz_common(const char *tz)
{
  return clk_set_tz_raw(PLATFORM_TZCMD_ALT, tz);
}

static uint8_t clock_set_tz_common(const char *tz)
{
  return clk_set_tz_raw(PLATFORM_TZCMD_MAIN, tz);
}
#endif /* UNUSED */

static uint8_t clock_get_tz_common(char *tz)
{
  uint8_t len;

  if (!platform_clk_get_tz_len(&len))
    return FN_ERR_IO_ERROR;
  return clk_result(CLKCALL_RV(APETIMECMD_GET_GENERAL, tz, len));
}

#endif /* FUJI_CLOCK_COMMON_H */

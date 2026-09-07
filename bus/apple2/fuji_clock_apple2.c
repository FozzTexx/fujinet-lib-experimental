#include "fuji_clock_common.h"
#ifdef UNUSED
#include <fujinet-clock.h>
#include <string.h>
#include <ctype.h>
#endif /* UNUSED */

#define PLATFORM_CLK_TIME_CALL(format, alt, buf, len) \
  CLKCALL_RV((alt) ? CLK_ALTIFYERIZE(clk_cmd[format]) : clk_cmd[format], buf, len)

static uint8_t clock_set_alternate_tz_apple2(const char *tz)
{
  return clk_result(CLKCALL_D(APETIMECMD_SETTZ_ALT, tz, strlen(tz) + 1));
}

uint8_t clock_set_tz_apple2(const char *tz)
{
  return clk_result(CLKCALL_D(APETIMECMD_SETTZ_ALT2, tz, strlen(tz) + 1));
}

uint8_t clock_get_tz_apple2(char *tz)
{
  return clk_result(CLKCALL_RV(APETIMECMD_GET_GENERAL, tz, 64));
}

uint8_t clock_get_time_apple2(uint8_t *time_data, TimeFormat format)
{
  return clock_get_time_common(time_data, format, false);
}

uint8_t clock_get_time_tz_apple2(uint8_t *time_data, const char *tz, TimeFormat format)
{
  return clock_get_time_tz_common(time_data, tz, format);
}
#define PLATFORM_TZCMD_ALT   APETIMECMD_SETTZ_ALT
#define PLATFORM_TZCMD_MAIN  APETIMECMD_SETTZ_ALT2

static bool platform_clk_set_tz_call(uint8_t cmd, const char *tz)
{
  return CLKCALL_D(cmd, tz, strlen(tz) + 1);
}

static bool platform_clk_get_tz_len(uint8_t *len_out)
{
  *len_out = 64;
  return true;
}

#include "fuji_clock_common.h"

static uint8_t clock_set_alternate_tz_apple2(const char *tz)
{
  return clock_set_alternate_tz_common(tz);
}

uint8_t clock_set_tz_apple2(const char *tz)
{
  return clock_set_tz_common(tz);
}

uint8_t clock_get_tz_apple2(char *tz)
{
  return clock_get_tz_common(tz);
}

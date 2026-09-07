#include "fujinet-clock.h"

#include <string.h>
#include <ctype.h>

static bool platform_clk_set_tz_call(uint8_t cmd, const char *tz)
{
  return CLKCALL_D(cmd, tz, strlen(tz) + 1);
}

static bool platform_clk_get_tz_len(uint8_t *len_out)
{
  /* Apple II firmware has no length-query command; the reply buffer is
   * always treated as 64 bytes. */
  *len_out = 64;
  return true;
}

#include "../../common/fuji_clock_common.h"

/* ---- public API ---- */

uint8_t clock_set_tz_apple2(const char *tz)
{
  return clock_set_tz_common(tz);
}

uint8_t clock_get_tz_apple2(char *tz)
{
  return clock_get_tz_common(tz);
}

uint8_t clock_get_time_apple2(uint8_t *time_data, TimeFormat format)
{
  return clock_get_time_common(time_data, format, false);
}

uint8_t clock_get_time_tz_apple2(uint8_t *time_data, const char *tz, TimeFormat format)
{
  return clock_get_time_tz_common(time_data, tz, format);
}

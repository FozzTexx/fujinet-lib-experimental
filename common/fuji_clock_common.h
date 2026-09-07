/**
 * @brief Shared implementation logic for fuji_clock.c / fuji_clock_apple2.c
 * @license gpl v. 3, see LICENSE for details.
 *
 * This header is textually included, once, near the top of each platform's
 * fuji_clock*.c file (after that file has defined the hooks below). It is
 * NOT meant to be included anywhere else, and is not part of the public API
 * in fujinet-clock.h.
 *
 * Before #include-ing this file, the platform .c file must:
 *
 *   #define PLATFORM_TZCMD_MAIN                  // command byte: persistent tz set
 *   #define PLATFORM_TZCMD_ALT                   // command byte: one-shot/alt tz set
 *   #define PLATFORM_CLK_TIME_CALL(format, alt, buf, len)   -> bool
 *
 * and implement, as `static`:
 *
 *   static bool platform_clk_set_tz_call(uint8_t cmd, const char *tz);
 *   static bool platform_clk_get_tz_len(uint8_t *len_out);
 *
 * PLATFORM_CLK_TIME_CALL and the two platform_clk_* functions express the
 * only real per-platform differences (wire calling convention, command
 * codes). Everything else -- bounds checking, null-terminating string
 * replies, translating a bool into an FN_ERR_* code, and the
 * "apply one-shot tz, then read the time" sequence -- is identical across
 * platforms and lives here exactly once.
 */

#ifndef FUJI_CLOCK_COMMON_H
#define FUJI_CLOCK_COMMON_H

#include "fujinet-clock.h"

extern const uint8_t clk_cmd[];
extern const uint8_t clk_reply_len[TIMEFORMAT_COUNT];

static uint8_t clk_result(bool ok)
{
  return ok ? FN_ERR_OK : FN_ERR_IO_ERROR;
}

/* ---- time ---- */

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

/* ---- timezone ---- */

static uint8_t clock_set_alternate_tz_common(const char *tz)
{
  return clk_result(platform_clk_set_tz_call(PLATFORM_TZCMD_ALT, tz));
}

static uint8_t clock_set_tz_common(const char *tz)
{
  return clk_result(platform_clk_set_tz_call(PLATFORM_TZCMD_MAIN, tz));
}

static uint8_t clock_get_tz_common(char *tz)
{
  uint8_t len;

  if (!platform_clk_get_tz_len(&len))
    return FN_ERR_IO_ERROR;
  return clk_result(CLKCALL_RV(APETIMECMD_GET_GENERAL, tz, len));
}

static uint8_t clock_get_time_tz_common(uint8_t *time_data, const char *tz, TimeFormat format)
{
  uint8_t rc;

  if ((uint8_t) format >= TIMEFORMAT_COUNT)
    return FN_ERR_BAD_CMD;

  /* NOTE: calls the platform's own static alt-tz setter directly, by its
   * real name. The original code called a bare `clock_set_alternate_tz(tz)`,
   * which fujinet-clock.h never declares or aliases for either platform --
   * that call had no matching declaration anywhere in this library. */
  rc = clock_set_alternate_tz_common(tz);
  if (rc != FN_ERR_OK)
    return rc;
  return clock_get_time_common(time_data, format, /*alt=*/true);
}

#endif /* FUJI_CLOCK_COMMON_H */

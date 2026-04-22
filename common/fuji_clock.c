#include "fujinet-clock.h"

#ifdef _CMOC_VERSION_
#include <cmoc.h>
#else
#include <string.h>
#endif

#define APETIMECMD_GETTIME      0x93
#define APETIMECMD_SETTZ        0x99

#if defined(BUILD_ATARI) || defined(BUILD_APPLE2) || defined(BUILD_COCO) || defined(BUILD_MSDOS)

#define TIMEFORMAT_COUNT        6

static const uint8_t clk_reply_len[TIMEFORMAT_COUNT] = { 7, 4, 6, 25, 25, 19 };

#if defined(BUILD_ATARI) || defined(BUILD_COCO) || defined(BUILD_MSDOS)
static const uint8_t clk_cmd[TIMEFORMAT_COUNT] = { 'T', 'P', APETIMECMD_GETTIME, 'I', 'Z', 'S' };
#else /* BUILD_APPLE2 */
static const uint8_t clk_cmd[TIMEFORMAT_COUNT]     = { 'T', 'P', 'A', 'I', 'Z', 'S' };
static const uint8_t clk_cmd_alt[TIMEFORMAT_COUNT] = { 't', 'p', 'a', 'i', 'z', 's' };
#endif

static uint8_t clock_set_alternate_tz(const char *tz)
{
  size_t len = strlen(tz) + 1;
#if defined(BUILD_ATARI) || defined(BUILD_COCO) || defined(BUILD_MSDOS)
  return CLKCALL_B12_D(APETIMECMD_SETTZ, len, tz, len) ? FN_ERR_OK : FN_ERR_IO_ERROR;
#else /* BUILD_APPLE2 */
  return CLKCALL_D('t', tz, len) ? FN_ERR_OK : FN_ERR_IO_ERROR;
#endif
}

#endif /* BUILD_ATARI || BUILD_APPLE2 || BUILD_COCO || BUILD_MSDOS */

uint8_t clock_set_tz(const char *tz)
{
#if defined(BUILD_ATARI) || defined(BUILD_COCO) || defined(BUILD_MSDOS)
  size_t len = strlen(tz) + 1;
  return CLKCALL_B12_D('t', len, tz, len) ? FN_ERR_OK : FN_ERR_IO_ERROR;
#elif defined(BUILD_APPLE2)
  return CLKCALL_D('T', tz, strlen(tz) + 1) ? FN_ERR_OK : FN_ERR_IO_ERROR;
#else
  (void) tz;
  return FN_ERR_NO_DEVICE;
#endif
}

uint8_t clock_get_tz(char *tz)
{
#if defined(BUILD_ATARI) || defined(BUILD_COCO) || defined(BUILD_MSDOS)
  uint8_t len;
  if (!CLKCALL_RV('L', &len, 1))
    return FN_ERR_IO_ERROR;
  return CLKCALL_RV('G', tz, len) ? FN_ERR_OK : FN_ERR_IO_ERROR;
#elif defined(BUILD_APPLE2)
  return CLKCALL_RV('G', tz, 64) ? FN_ERR_OK : FN_ERR_IO_ERROR;
#else
  (void) tz;
  return FN_ERR_NO_DEVICE;
#endif
}

uint8_t clock_get_time(uint8_t *time_data, TimeFormat format)
{
#if defined(BUILD_ATARI) || defined(BUILD_COCO) || defined(BUILD_MSDOS)
  if ((uint8_t) format >= TIMEFORMAT_COUNT)
    return FN_ERR_BAD_CMD;
  return CLKCALL_A1_RV(clk_cmd[format], 0, time_data, clk_reply_len[format])
         ? FN_ERR_OK : FN_ERR_IO_ERROR;
#elif defined(BUILD_APPLE2)
  if ((uint8_t) format >= TIMEFORMAT_COUNT)
    return FN_ERR_BAD_CMD;
  return CLKCALL_RV(clk_cmd[format], time_data, clk_reply_len[format])
         ? FN_ERR_OK : FN_ERR_IO_ERROR;
#else
  (void) time_data;
  (void) format;
  return FN_ERR_NO_DEVICE;
#endif
}

uint8_t clock_get_time_tz(uint8_t *time_data, const char *tz, TimeFormat format)
{
#if defined(BUILD_ATARI) || defined(BUILD_APPLE2) || defined(BUILD_COCO) || defined(BUILD_MSDOS)
  uint8_t rc;
  if ((uint8_t) format >= TIMEFORMAT_COUNT)
    return FN_ERR_BAD_CMD;
  rc = clock_set_alternate_tz(tz);
  if (rc != FN_ERR_OK)
    return rc;
#if defined(BUILD_ATARI) || defined(BUILD_COCO) || defined(BUILD_MSDOS)
  return CLKCALL_A1_RV(clk_cmd[format], 1, time_data, clk_reply_len[format])
         ? FN_ERR_OK : FN_ERR_IO_ERROR;
#else /* BUILD_APPLE2 */
  return CLKCALL_RV(clk_cmd_alt[format], time_data, clk_reply_len[format])
         ? FN_ERR_OK : FN_ERR_IO_ERROR;
#endif
#else
  (void) time_data;
  (void) tz;
  (void) format;
  return FN_ERR_NO_DEVICE;
#endif
}

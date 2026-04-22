#include "fujinet-clock.h"

#include <string.h>

#define TIMEFORMAT_COUNT        6

static const uint8_t clk_reply_len[TIMEFORMAT_COUNT] = { 7, 4, 6, 25, 25, 19 };

extern const uint8_t *clk_cmd;
extern const uint8_t *clk_cmd_alt;

static uint8_t clock_set_alternate_tz(const char *tz)
{
  return CLKCALL_D('t', tz, strlen(tz) + 1) ? FN_ERR_OK : FN_ERR_IO_ERROR;
}

uint8_t clock_set_tz(const char *tz)
{
  return CLKCALL_D('T', tz, strlen(tz) + 1) ? FN_ERR_OK : FN_ERR_IO_ERROR;
}

uint8_t clock_get_tz(char *tz)
{
  return CLKCALL_RV('G', tz, 64) ? FN_ERR_OK : FN_ERR_IO_ERROR;
}

uint8_t clock_get_time(uint8_t *time_data, TimeFormat format)
{
  if ((uint8_t) format >= TIMEFORMAT_COUNT)
    return FN_ERR_BAD_CMD;
  return CLKCALL_RV(clk_cmd[format], time_data, clk_reply_len[format])
         ? FN_ERR_OK : FN_ERR_IO_ERROR;
}

uint8_t clock_get_time_tz(uint8_t *time_data, const char *tz, TimeFormat format)
{
  uint8_t rc;
  if ((uint8_t) format >= TIMEFORMAT_COUNT)
    return FN_ERR_BAD_CMD;
  rc = clock_set_alternate_tz(tz);
  if (rc != FN_ERR_OK)
    return rc;
  return CLKCALL_RV(clk_cmd_alt[format], time_data, clk_reply_len[format])
         ? FN_ERR_OK : FN_ERR_IO_ERROR;
}

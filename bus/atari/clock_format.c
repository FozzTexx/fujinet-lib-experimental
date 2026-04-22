#include <fujinet-int.h>
#include <fujinet-commands.h>

#define CLK_CMD_SIMPLE_BINARY      'T'
#define CLK_CMD_PRODOS_BINARY      'P'
#define CLK_CMD_APETIME_BINARY     FUJICMD_APETIME_GETTIME
#define CLK_CMD_TZ_ISO_STRING      'I'
#define CLK_CMD_UTC_ISO_STRING     'Z'
#define CLK_CMD_APPLE3_SOS_BINARY  'S'

static const uint8_t _clk_cmd[] = {
  CLK_CMD_SIMPLE_BINARY,
  CLK_CMD_PRODOS_BINARY,
  CLK_CMD_APETIME_BINARY,
  CLK_CMD_TZ_ISO_STRING,
  CLK_CMD_UTC_ISO_STRING,
  CLK_CMD_APPLE3_SOS_BINARY
};

const uint8_t *clk_cmd = _clk_cmd;

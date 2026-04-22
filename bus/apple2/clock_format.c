#include <fujinet-int.h>

#define CLK_CMD_SIMPLE_BINARY          'T'
#define CLK_CMD_PRODOS_BINARY          'P'
#define CLK_CMD_APETIME_BINARY         'A'
#define CLK_CMD_TZ_ISO_STRING          'I'
#define CLK_CMD_UTC_ISO_STRING         'Z'
#define CLK_CMD_APPLE3_SOS_BINARY      'S'

#define CLK_CMD_ALT_SIMPLE_BINARY      't'
#define CLK_CMD_ALT_PRODOS_BINARY      'p'
#define CLK_CMD_ALT_APETIME_BINARY     'a'
#define CLK_CMD_ALT_TZ_ISO_STRING      'i'
#define CLK_CMD_ALT_UTC_ISO_STRING     'z'
#define CLK_CMD_ALT_APPLE3_SOS_BINARY  's'

static const uint8_t _clk_cmd[] = {
  CLK_CMD_SIMPLE_BINARY,
  CLK_CMD_PRODOS_BINARY,
  CLK_CMD_APETIME_BINARY,
  CLK_CMD_TZ_ISO_STRING,
  CLK_CMD_UTC_ISO_STRING,
  CLK_CMD_APPLE3_SOS_BINARY
};

static const uint8_t _clk_cmd_alt[] = {
  CLK_CMD_ALT_SIMPLE_BINARY,
  CLK_CMD_ALT_PRODOS_BINARY,
  CLK_CMD_ALT_APETIME_BINARY,
  CLK_CMD_ALT_TZ_ISO_STRING,
  CLK_CMD_ALT_UTC_ISO_STRING,
  CLK_CMD_ALT_APPLE3_SOS_BINARY
};

const uint8_t *clk_cmd     = _clk_cmd;
const uint8_t *clk_cmd_alt = _clk_cmd_alt;

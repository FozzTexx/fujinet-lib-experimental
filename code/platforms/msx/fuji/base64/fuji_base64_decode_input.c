#include <stdbool.h>
#include <stdint.h>
#include "fujinet-bus-msx.h"
#include "fujinet-fuji.h"
#include "fujinet-commands.h"


bool fuji_base64_decode_input(char *s, uint16_t len)
{
  bool err = FUJICALL_B12_D(FUJICMD_BASE64_DECODE_INPUT, len, s, len);
  return err;
}
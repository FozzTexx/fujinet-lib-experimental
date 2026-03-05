#include <stdbool.h>
#include <stdint.h>
#include "fujinet-bus-msx.h"


bool fuji_base64_decode_output(char *s, uint16_t len) 
{
  err = FUJICALL_B12_RV(FUJICMD_BASE64_DECODE_INPUT, length, s, len);
  return err;
}
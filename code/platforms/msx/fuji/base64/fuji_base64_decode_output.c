#include <stdbool.h>
#include <stdint.h>
#include "fujinet-bus-msx.h"
#include "fujinet-fuji.h"
#include "fujinet-commands.h"


bool fuji_base64_decode_output(char *s, uint16_t len) 
{
  bool err = FUJICALL_B12_RV(FUJICMD_BASE64_DECODE_OUTPUT, len, s, len);
  return err;
}
#include <stdbool.h>
#include <stdint.h>
#include "fujinet-bus-msx.h"
#include "fujinet-fuji.h"
#include "fujinet-commands.h"


bool fuji_base64_decode_length(unsigned long *len)
{
  bool err = FUJICALL_RV(FUJICMD_BASE64_DECODE_LENGTH, len, sizeof(len));
  return err;
}
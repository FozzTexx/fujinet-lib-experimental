#include <stdbool.h>
#include <stdint.h>
#include "fujinet-bus-msx.h"
#include "fujinet-fuji.h"
#include "fujinet-commands.h"


bool fuji_base64_encode_length(unsigned long *len)
{
  bool err = FUJICALL_RV(FUJICMD_BASE64_ENCODE_LENGTH, len, sizeof(len));
  return err;
}
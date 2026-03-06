#include <stdbool.h>
#include <stdint.h>
#include "fujinet-bus-msx.h"
#include "fujinet-functions-platform.h"


bool fuji_base64_encode_compute(void)
{
  bool err = FUJICALL(FUJICMD_BASE64_DECODE_COMPUTE);
  return err;
}


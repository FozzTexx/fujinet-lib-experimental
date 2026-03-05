#include <stdbool.h>
#include <stdint.h>
#include "fujinet-bus-msx.h"


bool fuji_base64_decode_compute(void);
{
  err = FUJICALL(FUJICMD_BASE64_DECODE_COMPUTE);
  return err;
}


#include <stdbool.h>
#include <stdint.h>
#include "fujinet-bus-msx.h"
#include "fujinet-fuji.h"
#include "fujinet-commands.h"


bool fuji_base64_decode_compute(void)
{
  bool err = FUJICALL(FUJICMD_BASE64_DECODE_COMPUTE);
  return err;
}


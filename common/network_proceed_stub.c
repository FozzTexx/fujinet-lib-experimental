#include <fujinet-network.h>

/* CoCo implements these in bus/coco/network_proceed.c using the PIA
   carrier-detect line. No other platform has a proceed line, so stub
   them out to keep the declared API linkable everywhere. */
#ifndef BUILD_COCO

bool network_has_proceed(void)
{
  return false;
}

bool network_proceed(void)
{
  return false;
}

#endif /* ! BUILD_COCO */

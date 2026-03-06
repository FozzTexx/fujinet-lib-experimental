#ifndef FUJINET_BUS_MSX_H
#define FUJINET_BUS_MSX_H

#include "fujinet-bus.h"

#define milliseconds_to_jiffy(millis) ((millis) / (VDP_IS_PAL ? 20 : 1000 / 60))

#define TIMEOUT         milliseconds_to_jiffy(100)
#define TIMEOUT_SLOW	milliseconds_to_jiffy(15 * 1000)
#define MAX_RETRIES	1

#endif /* FUJINET_BUS_MSX_H */

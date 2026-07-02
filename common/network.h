#ifndef NETWORK_UNIT_STATUS_DEFAULT_H
#define NETWORK_UNIT_STATUS_DEFAULT_H

#include <fujinet-int.h>
#include <fujinet-err.h>

#ifdef BUILD_APPLE
#warning "MAX_JSON_QUERY_LEN should be 256 but there are bugs in iwm implementation"
#define MAX_JSON_QUERY_LEN 255
#else /* ! BUILD_APPLE */
#define MAX_JSON_QUERY_LEN 256
#endif /* BUILD_APPLE */

typedef struct {
  uint16_t avail;
  uint8_t status;
  uint8_t errcode;
} NetworkStatus;

extern NetworkStatus nw_status;

#define network_unit_status(unit, nw_status) (!NETCALL_RV(FUJICMD_STATUS, unit, nw_status, sizeof(*nw_status)))

#endif /* NETWORK_UNIT_STATUS_DEFAULT_H */

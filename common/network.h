#ifndef NETWORK_UNIT_STATUS_DEFAULT_H
#define NETWORK_UNIT_STATUS_DEFAULT_H

#include <fujinet-int.h>
#include <fujinet-err.h>
#include <fujinet-network.h> /* NetworkStatus, nw_status */

#define MAX_JSON_QUERY_LEN 256

/* On DriveWire the 16-bit avail field is big-endian on the wire, but the
   6809 is big-endian too, so the struct read is naturally correct. */
#define network_unit_status(unit, nws) (!NETCALL_RV(FUJICMD_STATUS, unit, nws, sizeof(NetworkStatus)))

#endif /* NETWORK_UNIT_STATUS_DEFAULT_H */

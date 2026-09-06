#include "network.h"

#include <fujinet-network.h>

#if FUJI_VARIABLE_LEN_PACKETS
#include <string.h>
#endif /* FUJI_VARIABLE_LEN_PACKETS */

#if !defined(__ADAM__) && !defined(__COLECOADAM__)
int16_t network_json_query(const char *devicespec, const char *query, char *buffer)
{
  int16_t total, read_len;
  FN_ERR err;
  uint16_t avail;
  uint16_t query_len;
  uint8_t nw_unit = network_unit(devicespec);


#if FUJI_VARIABLE_LEN_PACKETS
  query_len = strlen(query);
  if (query_len > MAX_JSON_QUERY_LEN)
    query_len = MAX_JSON_QUERY_LEN;
#else
  /* SIO/DriveWire expect a fixed-size query frame */
  query_len = MAX_JSON_QUERY_LEN;
#endif /* FUJI_VARIABLE_LEN_PACKETS */

  if (!NETCALL_D(FUJICMD_QUERY, nw_unit, query, query_len))
    return -FN_ERR_IO_ERROR;

  total = 0;
  for (;;) {
    err = network_unit_status(nw_unit, &nw_status);
    if (err)
      return -err;

    avail = nw_status.avail;
    if (!avail)
      break;

    read_len = network_read(devicespec, &buffer[total], avail);
    if (read_len < 0)
      return read_len;

    total += read_len;
  }

  return network_json_strip_newlines(buffer, total);
}
#endif /* ! (__ADAM__ || __COLECOADAM__) */

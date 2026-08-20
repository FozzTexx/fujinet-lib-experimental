#include "fujinet-bus-adam.h"
#include "fujinet-network.h"

#include <stdio.h> // debug

int16_t network_read_nb_adam(const char *devicespec, void *buf, uint16_t len)
{
  uint8_t device, nw_unit;
  uint8_t status;
  DCB *dcb;
  uint16_t avail;
  uint8_t conn, nerr;


  nw_unit = network_unit(devicespec);
  device = fuji_remap_device(nw_unit + FUJI_DEVICEID_NETWORK - 1);
  if (!device)
    return 0;
  dcb = dcb_find(device);
  if (!dcb)
    return 0;

  for (;;) {
    status = dcb_io(dcb, DCB_COMMAND_READ, buf, len, MAX_RETRIES);
    if (status != DCB_STATUS_FINISH)
      return 0;

    if (dcb->len)
      return dcb->len;

    /* An empty read means the device NACKed, which it only does with
     * nothing staged, so asking for status here cannot discard queued
     * data. Status is the only thing that tells "no data yet" apart from
     * end of file, without which the caller's loop cannot terminate. */
    if (network_status(devicespec, &avail, &conn, &nerr) != FN_ERR_OK)
      return 0;

    if (nerr > NETWORK_SUCCESS && !avail)
      return 0;
  }
}

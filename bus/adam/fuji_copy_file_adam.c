#include "fujinet-bus-adam.h"
#include "fujinet-fuji.h"

/**
 * @brief Copy a file between host slots, waiting for the copy to finish.
 *
 * AdamNet completes a write once the packet has been delivered, not once
 * the device has acted on it, so the copy would otherwise return while the
 * FujiNet was still working and the commands that followed would be lost.
 * The CoCo waits the same way with FUJICMD_READY in its bus_ready().
 */
bool fuji_copy_file_adam(uint8_t src_slot, uint8_t dest_slot,
			 const char *copy_spec)
{
  DCB *dcb;


  if (!FUJICALL_A1_A2_D(FUJICMD_COPY_FILE, src_slot, dest_slot, copy_spec,
			MAX_FILENAME_LEN))
    return false;

  dcb = dcb_find(fuji_remap_device(FUJI_DEVICEID_FUJINET));
  if (!dcb)
    return false;

  return bus_ready(dcb, COPY_RETRIES);
}

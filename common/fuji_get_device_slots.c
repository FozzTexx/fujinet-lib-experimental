#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "fujinet-fuji.h"
#include "fujinet-bus.h"
#include "sp.h"

// d points to start of an array of device slots with length size
bool fuji_get_device_slots(DeviceSlot *d, size_t size)
{
	if (sp_get_fuji_id() == 0) {
		return false;
	}

	sp_error = sp_status(sp_fuji_id, FUJICMD_READ_DEVICE_SLOTS);
	if (sp_error == 0) {
		if (sp_count != sizeof(DeviceSlot) * size) {
			// didn't receive the correct amount of data for array we are filling
			sp_error = SP_ERR_IO_ERROR;
			return false;
		}
		memcpy(d, &sp_payload[0], sp_count);
	}
	return sp_error == 0;

}

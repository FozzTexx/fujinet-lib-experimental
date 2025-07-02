#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "fujinet-fuji.h"
#include "fujinet-bus.h"
#include "sp.h"

bool fuji_get_host_slots(HostSlot *h, size_t size)
{
	if (sp_get_fuji_id() == 0) {
		return false;
	}

	sp_error = sp_status(sp_fuji_id, FUJICMD_READ_HOST_SLOTS);
	if (sp_error == 0) {
		if (sp_count != sizeof(HostSlot) * size) {
			// didn't receive the correct amount of data for array we are filling
			sp_error = SP_ERR_IO_ERROR;
			return false;
		}
		memcpy(h, &sp_payload[0], sp_count);
	}
	return sp_error == 0;

}

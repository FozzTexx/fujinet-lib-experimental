#include <stdint.h>
#include "fujinet-fuji.h"
#include "fujinet-bus.h"

bool fuji_enable_device(uint8_t d)
{
	if (sp_get_fuji_id() == 0) {
		return false;
	}

	sp_payload[0] = 1;
	sp_payload[1] = 0;
	sp_payload[2] = d;

	sp_error = sp_control(sp_fuji_id, FUJICMD_ENABLE_DEVICE);
	return sp_error == 0;
}

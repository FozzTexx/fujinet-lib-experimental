#include <stdint.h>
#include "fujinet-fuji.h"
#include "fujinet-bus.h"

bool fuji_set_boot_config(uint8_t toggle)
{
    sp_error = sp_get_fuji_id();
	if (sp_error <= 0) {
		return false;
	}

	sp_payload[0] = 1;
	sp_payload[1] = 0;
	sp_payload[2] = toggle;

	sp_error = sp_control(sp_fuji_id, FUJICMD_CONFIG_BOOT);
	return sp_error == 0;
}

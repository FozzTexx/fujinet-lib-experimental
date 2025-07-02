#include <stdint.h>
#include "fujinet-fuji.h"
#include "fujinet-bus.h"

bool fuji_scan_for_networks(uint8_t *count)
{
	if (sp_get_fuji_id() == 0) {
		return false;
	}

	sp_error = sp_status(sp_fuji_id, FUJICMD_SCAN_NETWORKS);
	if (sp_error == 0) {
		*count = sp_payload[0];
	}
	return sp_error == 0;

}

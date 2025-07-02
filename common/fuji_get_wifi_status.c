#include <stdint.h>
#include "fujinet-fuji.h"
#include "fujinet-bus.h"

bool fuji_get_wifi_status(uint8_t *status)
{
	if (sp_get_fuji_id() == 0) {
		return false;
	}

	// IWM only currently returns either 3 (WL_CONNECTED), or 6 (WL_DISCONNECTED), but there are other codes.
	// If that changes in Firmware, nothing to do here, it'll automatically return new values.
	sp_error = sp_status(sp_fuji_id, FUJICMD_GET_WIFISTATUS);
	if (sp_error == 0) {
		*status = sp_payload[0];
	}

	return sp_error == 0;
}

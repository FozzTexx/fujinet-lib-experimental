#include <stdint.h>
#include <string.h>

#include "fujinet-fuji.h"
#include "fujinet-bus.h"

bool fuji_copy_file(uint8_t src_slot, uint8_t dst_slot, char *copy_spec)
{
	if (sp_get_fuji_id() == 0) {
		return false;
	}

	sp_payload[0] = 0;
	sp_payload[1] = 2; // 512 bytes

	sp_payload[2] = src_slot;
	sp_payload[3] = dst_slot;
	strcpy((char *) &sp_payload[4], copy_spec);

	sp_error = sp_control(sp_fuji_id, FUJICMD_COPY_FILE);
	return sp_error == 0;
}

#include <stdbool.h>
#include <stdint.h>
#include "fujinet-fuji.h"
#include "bus.h"

// Use 0x, and NOT string literals to avoid CC65 charmap translations. These are bytes not chars.
// All commands are <01><cmd>, we reuse this buffer to send all data, and only have to set 2nd byte
uint8_t cmd_args[2] = { 0x01, 0x00 };

bool open_or_write(uint8_t cmd)
{
	int bytes_written;
	uint8_t err_code = 0;

	cmd_args[1] = cmd;
	if (fuji_is_open) {
		// this is a continuation, so use the existing channel and write the data instead of 
		bytes_written = cbm_write(CBM_CMD_CHANNEL, cmd_args, 2);
		if (bytes_written != 2) {
			err_code = ERROR_OPEN_WRITE_CMD_FAILED;
		}
	} else {
		if (fuji_cbm_open(CBM_CMD_CHANNEL, CBM_DEV_FUJI, CBM_CMD_CHANNEL, 2, cmd_args) != 0) {
			err_code = ERROR_OPEN_CMD_FAILED;
		}
	}
	if (err_code != 0) {
		status_error(err_code, cmd);
		cbm_close(CBM_CMD_CHANNEL);
		fuji_is_open = false;
		return false;
	}
	return true;
}

// no parameters in this version, we just fetch results after it's executed
bool open_read_close(uint8_t cmd, bool should_close, int *bytes_read, uint16_t result_size, uint8_t *result_data)
{
	int bytes_written;

	if (!open_or_write(cmd)) {
		*bytes_read = 0;
		return false;
	}

	*bytes_read = cbm_read(CBM_CMD_CHANNEL, result_data, result_size);
	return get_fuji_status(should_close);
}
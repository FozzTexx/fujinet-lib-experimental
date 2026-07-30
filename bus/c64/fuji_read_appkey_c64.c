#include "fujinet-fuji.h"
#include "fujinet-cbm.h"

FNStatus _fuji_status;

extern void init_appkey(uint8_t key_id, uint8_t mode);

static uint8_t cmd_args[2];
static bool fuji_is_open = false;

bool open_or_write(uint8_t cmd, uint8_t opcode)
{
  int bytes_written;
  bool success = true;


  cmd_args[0] = opcode;
  cmd_args[1] = cmd;
  if (fuji_is_open) {
    // this is a continuation, so use the existing channel and write the data instead of
    bytes_written = cbm_write(CBM_CMD_CHANNEL, cmd_args, 2);
    if (bytes_written != 2) {
      success = false;
    }
  }
  else {
    if (fuji_cbm_open(CBM_CMD_CHANNEL, CBM_DEV_FUJI, CBM_CMD_CHANNEL, 2, cmd_args) != 0) {
      success = false;
    }
  }

  if (!success) {
    cbm_close(CBM_CMD_CHANNEL);
    fuji_is_open = false;
    return false;
  }

  return true;
}

// Use 0x, and NOT string literals to avoid CC65 charmap translations. These are bytes not chars.
uint8_t fuji_status_cmd[2] = { OPCODE_NO_PAYLOAD, FUJICMD_STATUS };

// An internal version of fuji_status called at the end of the open_ commands
// which uses the currently open connection to write the status command to.
// This function also closes the connection if requested to, which allows multi stage commands to do them all on single open to reduce transfers
bool get_fuji_status(bool should_close)
{
  int bytes_read;
  int bytes_written;

  // do a status call to find out if anything went wrong. Using the current open channel, so just write our bytes
  bytes_written = cbm_write(CBM_CMD_CHANNEL, fuji_status_cmd, 2);
  if (bytes_written != 2) {
    // always close on an error
    cbm_close(CBM_CMD_CHANNEL);
    fuji_is_open = false;
    return false;
  }

  bytes_read = cbm_read(CBM_CMD_CHANNEL, &_fuji_status.raw[0], sizeof(FNStatus));
  if (should_close) {
    cbm_close(CBM_CMD_CHANNEL);
    fuji_is_open = false;
  }

  // return true if the error is 0 (i.e. no error)
  return _fuji_status.value.error == 0;
}

// this is a command that has no return data, so just push command, its data, and read the status
bool open_close_data(uint8_t cmd, bool should_close, uint16_t params_size, uint8_t *cmd_params)
{
  int bytes_written;
  bool is_success;

  if (!open_or_write(cmd, OPCODE_HAS_PAYLOAD)) {
    return false;
  }

  fuji_is_open = true;

  // write the cmd parameters
  bytes_written = cbm_write(CBM_CMD_CHANNEL, cmd_params, params_size);

  // we only use is_success if the write succeeded. We have to get the status either way.
  // so just store the is_success value and then decide whether to use it or not.
  is_success = get_fuji_status(should_close);

  if (bytes_written != params_size) {
    // write failed, this is an out and out failure. The _fuji_status values will hold error strings etc.
    // force a close if it wouldn't have happened in the status
    if (!should_close)
      cbm_close(CBM_CMD_CHANNEL);
    fuji_is_open = false;
    return false;
  }

  return is_success;

}

// no parameters in this version, we just fetch results after it's executed
bool open_read_close(uint8_t cmd, bool should_close, int *bytes_read,
                     uint16_t result_size, uint8_t *result_data)
{
  if (!open_or_write(cmd, OPCODE_NO_PAYLOAD)) {
    *bytes_read = 0;
    return false;
  }

  *bytes_read = cbm_read(CBM_CMD_CHANNEL, result_data, result_size);
  return get_fuji_status(should_close);
}

bool fuji_read_appkey_c64(uint8_t key_id, uint16_t *count, uint8_t *data)
{
  uint8_t mode = 0;
  uint16_t buffer_size = 64; // TODO: key sizes overhaul
  int bytes_read;
  bool is_success;
  extern void appkey;

  if (ak_creator_id == 0) {
    return false;
  }

  init_appkey(key_id, 0);

  // send the creator / app / mode values
  if (!open_close_data(FUJICMD_OPEN_APPKEY, false, 6, &appkey)) {
    *count = 0;
    return false;
  }

  // creator data sent fine, read the key
  is_success = open_read_close(FUJICMD_READ_APPKEY, true, &bytes_read, buffer_size, data);
  *count = (bytes_read >= 0) ? bytes_read : 0;
  cbm_close(CBM_CMD_CHANNEL);

  return is_success;
}

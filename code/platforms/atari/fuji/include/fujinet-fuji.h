#ifndef FUJINET_FUJI_H
#define FUJINET_FUJI_H

#include "fujinet-fuji-base.h"


typedef struct
{
    uint16_t numSectors;
    uint16_t sectorSize;
    uint8_t hostSlot;
    uint8_t deviceSlot;
    char filename[256];
} NewDisk;


/**
 * @brief Sends the device/host/mode information for devices to FN
 * @return success status of request.
 */
#define fuji_set_device_filename(mode, hs, ds, buffer) FUJICALL_A1_A2_D(FUJICMD_SET_DEVICE_FULLPATH, ds, (hs << 4) | (mode), buffer, MAX_FILENAME_LEN)

/**
 * @brief Fetch the current HSIO index value.
 * @return success status of request
 */
bool fuji_get_hsio_index(uint8_t *index);

/**
 * @brief Sets HSIO speed index
 * @return success status of request.
 */
bool fuji_set_hsio_index(bool save, uint8_t index);

/**
 * @brief Sets SIO external clock speed
 * @return success status of request.
 */
bool fuji_set_sio_external_clock(uint16_t rate);


#endif /* FUJINET_FUJI_H */

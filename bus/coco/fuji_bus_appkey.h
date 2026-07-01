#ifndef FUJI_BUS_APPKEY_H
#define FUJI_BUS_APPKEY_H

#include <fujinet-int.h>
#include <fujinet-const.h>

typedef struct {
  uint16_t length;
  uint8_t data[MAX_APPKEY_LEN];
} FNAppKeyString;

extern FNAppKeyString appkey_buf;

#endif /* FUJI_BUS_APPKEY_H */

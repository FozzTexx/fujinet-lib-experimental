#include <fujinet-int.h>

extern uint16_t pack_payload(uint8_t *buf, uint8_t fields,
                             uint8_t aux1, uint8_t aux2, uint8_t aux3, uint8_t aux4,
                             const void *data, size_t data_length);

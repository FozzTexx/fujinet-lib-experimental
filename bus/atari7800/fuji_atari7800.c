#include "fujinet-bus-atari7800.h"
#include "fujinet-atari7800.h"

bool fuji_a7800_present(void)
{
  return FN_MAGIC0 == 'F' && FN_MAGIC1 == 'N' && FN_PROTOVER == FN_PROTO_VER;
}

uint8_t fuji_a7800_boot_state(void)
{
  return FN_BOOTSTAT;
}

uint8_t fuji_a7800_boot_percent(void)
{
  return FN_BOOTPCT;
}

uint8_t fuji_a7800_boot_error(void)
{
  return FN_BOOTERR;
}

/* The cart writes these a byte at a time during the push; two matching
   reads in a row cannot catch a carry half-propagated. */
static uint32_t count24(volatile uint8_t *p)
{
  uint32_t a, b;

  b = (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16);
  do {
    a = b;
    b = (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16);
  } while (a != b);

  return a;
}

uint32_t fuji_a7800_boot_got(void)
{
  return count24(FN_BOOTGOT);
}

uint32_t fuji_a7800_boot_total(void)
{
  return count24(FN_BOOTTOT);
}

uint8_t fuji_a7800_mode(void)
{
  return FN_MODE;
}

uint8_t fuji_a7800_handover(void)
{
  return FN_HANDOVER;
}

uint8_t fuji_a7800_staged(void)
{
  return FN_STAGED;
}

uint8_t fuji_a7800_tv(void)
{
  return FN_TV;
}

void fuji_a7800_set_tv(uint8_t tv)
{
  fn_regwr(FNR_TV, tv);
}

uint8_t fuji_a7800_inptctrl(void)
{
  return FN_INPTCTRL;
}

bool fuji_a7800_inpt_locked(void)
{
  return FN_INPTLOCK != 0;
}

uint8_t fuji_a7800_hsc(void)
{
  return FN_HSC;
}

void fuji_a7800_hsc_op(uint8_t op)
{
  fn_regwr(FNR_HSC, op);
}

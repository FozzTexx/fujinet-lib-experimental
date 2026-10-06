#include "fujinet-bus-sms.h"
#include "fujinet-sms.h"

bool fuji_sms_present(void)
{
  return FN_MAGIC0 == 'F' && FN_MAGIC1 == 'N' && FN_PROTOVER == FN_PROTO_VER;
}

uint8_t fuji_sms_boot_state(void)
{
  return FN_BOOTSTAT;
}

uint8_t fuji_sms_boot_percent(void)
{
  return FN_BOOTPCT;
}

uint8_t fuji_sms_boot_error(void)
{
  return FN_BOOTERR;
}

uint8_t fuji_sms_mode(void)
{
  return FN_MODE;
}

void fuji_sms_boot(void)
{
  fn_regwr(FNR_BOOTLOCK, FN_BOOTLOCK_MAGIC);
#asm
  di
  jp  0xB800
#endasm
}

void fuji_sms_exit_to_config(void)
{
#asm
  di
  jp  0xB803
#endasm
}

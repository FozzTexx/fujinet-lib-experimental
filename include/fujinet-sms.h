#ifndef FUJINET_SMS_H
#define FUJINET_SMS_H

/*
  Sega Master System facilities the cross-platform API has no place for.

  A FujiNet client is a z88dk +sms image that leaves $B000-$BFFF alone in
  every bank from 2 up (the cart's arena sits there over slot 2) and carries
  the "FUJI" claim at $7FDC; makefiles/platforms/sms.mk stamps and checks
  both. It may be any size up to 1 MB and bank slot 2 through $FFFF.
*/

#include <fujinet-int.h>

/* fuji_sms_boot_state() values. */
#define FUJI_SMS_BOOT_IDLE   0
#define FUJI_SMS_BOOT_XFER   1
#define FUJI_SMS_BOOT_READY  2
#define FUJI_SMS_BOOT_FAILED 0x80

/* fuji_sms_mode() values: what the cart is serving. */
#define FUJI_SMS_MODE_CONFIG 0
#define FUJI_SMS_MODE_GAME   1
#define FUJI_SMS_MODE_APP    2

/* Is a FujiNet cartridge underneath us, speaking a protocol we know? */
extern bool fuji_sms_present(void);

/*
  fuji_mount_disk_image() pushes the image to the cart while the call is
  still outstanding; poll fuji_sms_boot_state() for READY (or FAILED, with
  fuji_sms_boot_error() saying why). fuji_sms_boot_percent() runs 0-100.
*/
extern uint8_t fuji_sms_boot_state(void);
extern uint8_t fuji_sms_boot_percent(void);
extern uint8_t fuji_sms_boot_error(void);
extern uint8_t fuji_sms_mode(void);

/* Load the pushed image into the cart's SRAM and start it. Does not return;
   only meaningful once fuji_sms_boot_state() reads READY. */
extern void fuji_sms_boot(void);

/* Hand the console back to CONFIG. Does not return. */
extern void fuji_sms_exit_to_config(void);

#endif /* FUJINET_SMS_H */

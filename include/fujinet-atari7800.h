#ifndef FUJINET_ATARI7800_H
#define FUJINET_ATARI7800_H

/*
  Atari 7800 facilities the cross-platform API has no place for.

  A FujiNet client is a 32K image at $8000-$FFFF built through
  makefiles/platforms/atari7800.mk: its crt0, its linker config (the cart's
  16K of RAM at $4000 holds DATA, BSS, the heap and the C stack) and
  atari7800-romstamp.py, which checks the "FUJI" claim at $FF70, the
  mailbox's no-RMW rule and that nothing stores to $00-$1F.

  The lib's crt0 does not export _zonecounter, so cc65's clock() or its
  conio cursor code drags in cc65's own crt0 and the link fails.

  Never write $00-$1F or its mirrors: until locked, every TIA write replaces
  INPTCTRL, and a game can only be handed to the console's own BIOS while it
  is unlocked. So no TIA sound (the cart's POKEY is at $0450), no TIA WSYNC
  (MARIA's is at $24) and no INPTCTRL store. Reading $08-$0D is fine.
*/

#include <fujinet-int.h>

/* fuji_a7800_boot_state() values. */
#define FUJI_A7800_BOOT_IDLE   0
#define FUJI_A7800_BOOT_XFER   1
#define FUJI_A7800_BOOT_READY  2
#define FUJI_A7800_BOOT_FAILED 0x80

/* fuji_a7800_mode() values: what the cart is serving. */
#define FUJI_A7800_MODE_BOOT 0
#define FUJI_A7800_MODE_LOAD 1
#define FUJI_A7800_MODE_GAME 2
#define FUJI_A7800_MODE_APP  3

/* fuji_a7800_tv() values. */
#define FUJI_A7800_TV_NTSC 0
#define FUJI_A7800_TV_PAL  1

/* fuji_a7800_staged() bits: the image the last mount pushed. */
#define FUJI_A7800_STAGED_READY  0x01
#define FUJI_A7800_STAGED_CLAIM  0x02   /* a FujiNet app */
#define FUJI_A7800_STAGED_BIOSOK 0x04   /* this console's BIOS accepts it */
#define FUJI_A7800_STAGED_HSCROM 0x08   /* the High Score Cart ROM */

/* fuji_a7800_hsc() bits. */
#define FUJI_A7800_HSC_ROM   0x01   /* an HSC ROM is installed */
#define FUJI_A7800_HSC_ON    0x02   /* games get the HSC */
#define FUJI_A7800_HSC_SD    0x04   /* the last save reached the SD card */
#define FUJI_A7800_HSC_DIRTY 0x08   /* unsaved high scores */

/* fuji_a7800_hsc_op() operations. */
#define FUJI_A7800_HSCOP_OFF     0
#define FUJI_A7800_HSCOP_ON      1
#define FUJI_A7800_HSCOP_FORGET  2
#define FUJI_A7800_HSCOP_INSTALL 3  /* the staged HSC ROM becomes the HSC */

/* fuji_a7800_handover() values: how the last image was started. */
#define FUJI_A7800_HO_NONE   0
#define FUJI_A7800_HO_BIOS   1
#define FUJI_A7800_HO_DIRECT 2

/* Seconds before fn_commit() gives up (at least the cart's own budget for
   that call), and the last failed call's reason (the cart's error, 0xFF if
   it never answered, 144 on a NAK), named as in fujinet-lib's SIO targets. */
extern uint8_t fn_default_timeout;
extern uint8_t fn_device_error;

/* Is a FujiNet cartridge underneath us, speaking a protocol we know? */
extern bool fuji_a7800_present(void);

/*
  fuji_mount_disk_image() makes the FujiNet push the image to the cart; the
  push may finish before or after the call returns, so wait for the state
  to read READY (or FAILED, with fuji_a7800_boot_error() saying why) before
  booting. While it runs, percent runs 0-100, and got and total are byte
  counts (total is 0 until the stream opens).
*/
extern uint8_t fuji_a7800_boot_state(void);
extern uint8_t fuji_a7800_boot_percent(void);
extern uint8_t fuji_a7800_boot_error(void);
extern uint32_t fuji_a7800_boot_got(void);
extern uint32_t fuji_a7800_boot_total(void);

extern uint8_t fuji_a7800_mode(void);
extern uint8_t fuji_a7800_handover(void);
extern uint8_t fuji_a7800_staged(void);

/* The cart's TV standard: PAL if the console's BIOS ran from $C000, or as
   set. fuji_a7800_detect_tv() counts MARIA's lines for itself, tells the
   cart, and returns FUJI_A7800_TV_*; it takes about two frames. */
extern uint8_t fuji_a7800_tv(void);
extern void fuji_a7800_set_tv(uint8_t tv);
extern uint8_t fuji_a7800_detect_tv(void);

/* The cart's model of INPTCTRL, and whether something has locked it. */
extern uint8_t fuji_a7800_inptctrl(void);
extern bool fuji_a7800_inpt_locked(void);

/* The High Score Cart: FUJI_A7800_HSC_* bits, and FUJI_A7800_HSCOP_*. */
extern uint8_t fuji_a7800_hsc(void);
extern void fuji_a7800_hsc_op(uint8_t op);

/* The NMI vector points at this JMP in RAM; a display-list interrupt
   handler stores its own address in bytes 1-2. The crt0 aims it at an RTI. */
extern uint8_t fuji_a7800_nmi[3];

/*
  Leave for good: interrupts and MARIA's DMA off, the POKEY quiet and the
  joystick ports back to inputs, then into the cartridge's loader.
  fuji_a7800_boot() starts the staged image (once fuji_a7800_boot_state()
  reads READY); fuji_a7800_config() reloads CONFIG.
*/
extern void fuji_a7800_boot(void);
extern void fuji_a7800_config(void);

#endif /* FUJINET_ATARI7800_H */

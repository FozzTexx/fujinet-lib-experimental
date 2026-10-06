#ifndef FUJINET_BUS_SMS_H
#define FUJINET_BUS_SMS_H

/*
  The Sega Master System's half of the FujiNet cartridge mailbox.

  Mirrors fujinet-firmware/pico/sms/firmware/include/fuji_mailbox.h by hand,
  which is the source of truth; keep the two in step. The cart edge has /WR,
  so console -> cart is an ordinary store to a hotspot page (one store per
  register write, a store anywhere in the TX page appends a byte) and the
  reply is memory the cart paints, read in place. The 4K arena sits at
  $B000-$BFFF, over the top of slot 2 in every bank.
*/

#include <fujinet-bus.h>

/* ---- cart -> console ---- */
#define FN_REPLY     ((volatile uint8_t *) 0xB000)
#define FN_REPLY_MAX 1024

#define FN_ACKSEQ    (*(volatile uint8_t *) 0xB400)
#define FN_STATUS    (*(volatile uint8_t *) 0xB401)
#define FN_ERRCODE   (*(volatile uint8_t *) 0xB402)
#define FN_REPLYCMD  (*(volatile uint8_t *) 0xB403)
#define FN_RXLEN_LO  (*(volatile uint8_t *) 0xB404)
#define FN_RXLEN_HI  (*(volatile uint8_t *) 0xB405)
#define FN_BOOTSTAT  (*(volatile uint8_t *) 0xB406)
#define FN_BOOTPCT   (*(volatile uint8_t *) 0xB407)
#define FN_BOOTERR   (*(volatile uint8_t *) 0xB408)
#define FN_MAGIC0    (*(volatile uint8_t *) 0xB409)
#define FN_MAGIC1    (*(volatile uint8_t *) 0xB40A)
#define FN_PROTOVER  (*(volatile uint8_t *) 0xB40B)
#define FN_MODE      (*(volatile uint8_t *) 0xB412)
#define FN_LINK      (*(volatile uint8_t *) 0xB415)
#define FN_BOOTGOT   ((volatile uint8_t *) 0xB416)    /* 24-bit LE */
#define FN_BOOTTOT   ((volatile uint8_t *) 0xB419)    /* 24-bit LE */

#define FN_STATUS_LINK 0x01
#define FN_PROTO_VER   1

/* ---- console -> cart: stores ---- */
#define FN_REGSEL    0xB500
#define FN_TXPAGE    (*(volatile uint8_t *) 0xB700)

#define FNR_DEVICE   0x00
#define FNR_CMD      0x01
#define FNR_NPARAM   0x02
#define FNR_DATA_RST 0x05
#define FNR_RXSLICE  0x06
#define FNR_SEQ      0x10
#define FNR_BOOTLOCK 0x11

#define FN_BOOTLOCK_MAGIC 0xB5
#define FN_LOADER_BOOT    0xB800
#define FN_LOADER_CONFIG  0xB803

/* NPARAM x { size 1|2|4, value LE }, then the payload. */
#define FN_TX_MAX    320

/* fn_commit() results: the cart's own error codes, plus our own timeout. */
#define FN_OK        0
#define FN_ENOLINK   1
#define FN_ETIMEOUT  2
#define FN_EBADFRAME 3
#define FN_ETOOBIG   4
#define FN_EWAIT     0xFF  /* the cart never answered at all */

/* Length of the reply left in the window by the last fuji_bus_call(). */
extern uint16_t fuji_bus_call_rlen;

extern void fn_regwr(uint8_t reg, uint8_t val);
extern uint8_t fn_commit(void);

#endif /* FUJINET_BUS_SMS_H */

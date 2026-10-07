#ifndef FUJINET_BUS_ATARI7800_H
#define FUJINET_BUS_ATARI7800_H

/*
  The Atari 7800's half of the FujiNet cartridge mailbox.

  Mirrors fujinet-firmware/pico/atari-7800/firmware/include/fuji_mailbox.h by
  hand, which is the source of truth; keep the two in step. The edge has R/W,
  so console -> cart is an ordinary store to a hotspot page and the reply is
  memory the cart paints, read in place. The arena sits at $0800-$0FFF, below
  the cart's own $4000-$FFFF, with the loader just under it at $0600.

  Stores to $0D00-$0FFF must be plain STA/STX/STY: a read-modify-write
  instruction writes the old value first, which the cart sees as an event.
  In C, never `++` or `|=` a mailbox address. atari7800-romstamp.py rejects
  the opcodes in a built image.
*/

#include <fujinet-bus.h>

/* ---- cart -> console: painted memory ---- */
#define FN_REPLY     ((volatile uint8_t *) 0x0800)
#define FN_REPLY_MAX 1024

#define FN_ACKSEQ    (*(volatile uint8_t *) 0x0C00)
#define FN_STATUS    (*(volatile uint8_t *) 0x0C01)
#define FN_ERRCODE   (*(volatile uint8_t *) 0x0C02)
#define FN_REPLYCMD  (*(volatile uint8_t *) 0x0C03)
#define FN_RXLEN_LO  (*(volatile uint8_t *) 0x0C04)
#define FN_RXLEN_HI  (*(volatile uint8_t *) 0x0C05)
#define FN_BOOTSTAT  (*(volatile uint8_t *) 0x0C06)
#define FN_BOOTPCT   (*(volatile uint8_t *) 0x0C07)
#define FN_BOOTERR   (*(volatile uint8_t *) 0x0C08)
#define FN_MAGIC0    (*(volatile uint8_t *) 0x0C09)
#define FN_MAGIC1    (*(volatile uint8_t *) 0x0C0A)
#define FN_PROTOVER  (*(volatile uint8_t *) 0x0C0B)
#define FN_MODE      (*(volatile uint8_t *) 0x0C12)
#define FN_MAPPER    (*(volatile uint8_t *) 0x0C14)
#define FN_LINK      (*(volatile uint8_t *) 0x0C15)
#define FN_BOOTGOT   ((volatile uint8_t *) 0x0C16)    /* 24-bit LE */
#define FN_BOOTTOT   ((volatile uint8_t *) 0x0C19)    /* 24-bit LE */
#define FN_INPTCTRL  (*(volatile uint8_t *) 0x0C1C)
#define FN_INPTLOCK  (*(volatile uint8_t *) 0x0C1D)
#define FN_TV        (*(volatile uint8_t *) 0x0C1E)
#define FN_STAGED    (*(volatile uint8_t *) 0x0C1F)
#define FN_STAGEDKIND (*(volatile uint8_t *) 0x0C20)
#define FN_STAGEDCRC ((volatile uint8_t *) 0x0C21)    /* 32-bit LE */
#define FN_HANDOVER  (*(volatile uint8_t *) 0x0C25)
#define FN_HSC       (*(volatile uint8_t *) 0x0C26)

#define FN_STATUS_LINK 0x01
#define FN_PROTO_VER   1

/* ---- console -> cart: stores ---- */
#define FN_REGSEL    ((volatile uint8_t *) 0x0D00)    /* [reg] = value */
#define FN_TXPAGE    (*(volatile uint8_t *) 0x0F00)   /* = byte: append */

#define FNR_DEVICE   0x00
#define FNR_CMD      0x01
#define FNR_NPARAM   0x02
#define FNR_DATA_RST 0x05
#define FNR_RXSLICE  0x06
#define FNR_SEQ      0x10
#define FNR_BOOTLOCK 0x11
#define FNR_TV       0x15
#define FNR_HSC      0x16

#define FN_BOOTLOCK_MAGIC 0xB5

/* The loader's entry points; jump, never call. */
#define FN_LOADER_BOOT   0x0600
#define FN_LOADER_CONFIG 0x0603

/* NPARAM x { size 1|2|4, value LE }, then the payload. */
#define FN_TX_MAX    320

/* fn_commit() results: the cart's own error codes, plus our own timeout. */
#define FN_OK        0
#define FN_ENOLINK   1
#define FN_ETIMEOUT  2
#define FN_EBADFRAME 3
#define FN_ETOOBIG   4
#define FN_EWAIT     0xFF  /* the cart never answered at all */

/* One register write is one store; one TX byte is one store. Functions, so
   every mailbox store in a program is a plain `sta`. */
extern void __fastcall__ fn_tx(uint8_t b);
extern void fn_regwr(uint8_t reg, uint8_t val);
extern uint8_t fn_commit(void);

/* The last fuji_bus_call()'s reply length, clamped to its buffer when it
   copied one. */
extern uint16_t fuji_bus_call_rlen;

#endif /* FUJINET_BUS_ATARI7800_H */

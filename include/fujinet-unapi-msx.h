#ifndef FUJINET_UNAPI_MSX_H
#define FUJINET_UNAPI_MSX_H

/* The MSX-UNAPI contract for reaching a FujiNet.
 *
 * This is the interface an implementation publishes and a client calls, so it
 * is shared between this library and the ROM side (the FujiNet MSX UNAPI
 * firmware, and the implementation carried in the Nextor driver bank).  Keep
 * the two in step.
 */

#include <fujinet-int.h>

/* The API identifier an implementation answers to during EXTBIO discovery.
   Identifiers are case insensitive. */
#define FUJI_UNAPI_ID "FUJINET"

/* The block routines FUJI_CALL_WRITE and FUJI_CALL_READ take in HL.
 *
 * The specification requires that a caller keep this block - and the buffer it
 * points at - out of page 1 and page 2: an implementation is free to page
 * itself in over either while the call runs.  Page 3 is the safe place.
 */
typedef struct {
  uint8_t device;
  uint8_t command;
  uint8_t aux_descr;
  uint8_t aux1, aux2, aux3, aux4;
  void *buffer;
  uint16_t length;
} FujiNetParams;

/* Routine numbers, from the FujiNet Firmware UNAPI specification 1.0.  The
   specification counts four routines but assigns nothing to 1, so 1 is left
   out here rather than renumbering the two that follow it. */
enum {
  FUJI_CALL_INFO  = 0,
  FUJI_CALL_WRITE = 2,
  FUJI_CALL_READ  = 3,
};

/* How fuji_bus_call() reaches the implementation.
 *
 * Splitting this out is what lets the same library serve two very different
 * callers.  A program that is separate from the FujiNet gets the back end in
 * bus/msx/unapi/, which finds an implementation with EXTBIO and enters it with
 * an inter-slot call.  Code built *into* an implementation cannot do that - an
 * inter-slot call switches slots but not ROM banks, so it would re-enter its
 * own cartridge at the wrong bank - and supplies its own definition instead,
 * calling the routines directly.  Whichever definition is in the link wins;
 * the library module is only pulled in when nothing else defines this.
 *
 * Out: 1 if the device answered, 0 if it did not.
 */
extern uint8_t fuji_unapi_call(uint8_t func, FujiNetParams *params);

#endif /* FUJINET_UNAPI_MSX_H */

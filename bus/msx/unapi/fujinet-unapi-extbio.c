/* The stand-off UNAPI back end: find a FujiNet implementation somewhere in the
 * machine and enter it with an inter-slot call.
 *
 * This is the right back end for a program that is separate from the FujiNet -
 * an ordinary cartridge, a .COM, a BASIC extension in its own ROM.  Code built
 * *into* an implementation must not use it: an inter-slot call switches slots
 * but leaves the ROM bank register alone, so entering your own cartridge this
 * way lands at the published address in whichever bank happens to be selected.
 * Such a build supplies its own fuji_unapi_call() and this module is then never
 * pulled out of the library.
 */

#include "fujinet-unapi-msx.h"

/* Discovery result, kept between calls.  unapi_entry doubles as the "have we
   looked yet" flag: an implementation's entry point is in page 1 or 2 and so
   is never zero. */
static uint8_t unapi_slot;
static uint8_t unapi_in_ram;
static void *unapi_entry;

static uint8_t unapi_count;
static uint8_t unapi_result;
static uint8_t call_func;
static void *call_arg;

/* How many implementations of the API named at ARG are installed.
   In: DE = 2222h, A = 0, B = 0.  Out: B = count. */
static uint8_t UNAPIGetCount(void)
{
  const char *service = FUJI_UNAPI_ID;
  volatile char *p;
  volatile char *ARG = (volatile char *) 0xF847;


  for (p = ARG; *service; p++, service++)
    *p = *service;
  *p = 0;

  __asm
    push ix
    push iy
    ld   de, 0x2222
    xor  a
    ld   b, a
    call 0xFFCA         ; EXTBIO
    ld   a, b
    ld   (_unapi_count), a
    pop  iy
    pop  ix
    __endasm;

  return unapi_count;
}

/* Where the first of them is.  In: DE = 2222h, A = 1, B = 1.
   Out: A = slot, B = segment (FFh if not in a mapped RAM segment),
        HL = entry point.

   Index 1 rather than a parameter: any implementation of this API will do, and
   the first one is as good as any. */
static void UNAPIGetSlot(void)
{
  __asm
    push ix
    push iy
    ld   de, 0x2222
    ld   a, 1           ; implementation index
    ld   b, 1           ; function 1: where is it
    call 0xFFCA         ; EXTBIO
    ld   (_unapi_slot), a
    ld   (_unapi_entry), hl
    ld   a, b
    ld   (_unapi_in_ram), a
    pop  iy
    pop  ix
    __endasm;
}

/* Enter the implementation through CALSLT, which wants the routine address in
   IX and the slot in IYh.  The arguments come out of memory rather than off the
   stack so that nothing here depends on the compiler's calling convention.

   In: A = routine number, HL = parameter block.  Out: L = 1 ok, 0 not. */
static uint8_t UNAPICall(void)
{
  __asm
    push ix
    push iy

    ld   a, (_call_func)
    ld   hl, (_call_arg)

    push af             ; the routine number, while A is needed for the slot
    ld   ix, (_unapi_entry)
    ld   a, (_unapi_slot)
    push af
    pop  iy             ; IYh = slot; CALSLT ignores IYl
    pop  af

    call 0x001C         ; CALSLT

    ld   a, l           ; only L carries the result; H is undefined
    ld   (_unapi_result), a

    pop  iy
    pop  ix
    __endasm;

  return unapi_result;
}

uint8_t fuji_unapi_call(uint8_t func, FujiNetParams *params)
{
  if (!unapi_entry) {
    if (!UNAPIGetCount())
      return 0;
    UNAPIGetSlot();
#ifdef DEBUG
    printf("IN SLOT %d RAM: %d ENTRY: 0x%04x\n", unapi_slot, unapi_in_ram, unapi_entry);
#endif
    if (!unapi_entry)
      return 0;
  }

  call_func = func;
  call_arg = params;
  return UNAPICall();
}

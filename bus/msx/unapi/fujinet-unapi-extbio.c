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

#include <string.h>

/* How much stack to set aside for bouncing a caller's buffer out of a page the
   implementation is about to map itself over.  A caller that keeps its buffers
   where the specification says to never reaches this at all.
 
   Two sizes rather than one, because the frame is only free if it is not
   taken: a cartridge has its strings in ROM, which is page 1, so short
   transfers hit this on every open - and a cartridge's stack is the few
   hundred bytes between its BSS and HIMEM, which one big frame walks straight
   through.  The small one covers a URL or a filename; the large one is only
   entered by a caller that also put a whole data buffer in the way. */
#ifndef FUJI_UNAPI_SMALL_BOUNCE
#define FUJI_UNAPI_SMALL_BOUNCE 256
#endif
#ifndef FUJI_UNAPI_BOUNCE_SIZE
#define FUJI_UNAPI_BOUNCE_SIZE 1024
#endif

/* The two pages an implementation is free to take over while a call runs:
   CALSLT maps it into page 1 to enter it at all, and the FujiNet one pages
   itself into page 2 as well to reach its window.  Anything it dereferences -
   the parameter block, and the buffer that block points at - has to sit
   outside both, which leaves page 0 and page 3. */
#define IN_SWITCHED_PAGE(a) \
  ((uint16_t) (a) >= 0x4000 && (uint16_t) (a) < 0xC000)

/* The stack is where the bounce comes from, so it has to be page 3 itself.
   It is on every caller worth the name - MSX-DOS puts it at the top of the
   TPA and the BIOS puts it just under HIMEM - but a caller that has moved it
   somewhere else would be quietly corrupted rather than told, so it is
   checked rather than assumed. */
#define STACK_IS_SAFE(a) ((uint16_t) (a) >= 0xC000)

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

static uint8_t enter(uint8_t func, FujiNetParams *params)
{
  call_func = func;
  call_arg = params;
  return UNAPICall();
}

/* Copy back what the implementation may have changed.  Everything but the
   buffer pointer, which on the way in was ours and not the caller's. */
static void unbounce(FujiNetParams *params, FujiNetParams *local, void *buffer)
{
  memcpy(params, local, sizeof(*params));
  params->buffer = buffer;
}

/* The parameter block alone, moved to the stack.  This is the common case and
   the one the library used to get wrong: the block is a static, and in a ROM
   that lands in page 3 as the comment where it is declared says, but in an
   MSX-DOS .COM of any size BSS follows the program up out of page 0 and into
   page 1 - where the implementation is about to be. */
static uint8_t bounce_block(uint8_t func, FujiNetParams *params)
{
  FujiNetParams local;
  uint8_t ok;

  if (!STACK_IS_SAFE(&local))
    return enter(func, params); /* nowhere better to put it; no worse than not trying */

  memcpy(&local, params, sizeof(local));
  ok = enter(func, &local);
  unbounce(params, &local, params->buffer);
  return ok;
}

/* The block and the buffer it points at.  Only for a caller that put its
   buffer somewhere the specification does not allow; the copy is the price of
   not silently handing the implementation a pointer into its own ROM.  The
   scratch belongs to the caller so that the frame it costs is chosen there. */
static uint8_t bounce_via(uint8_t func, FujiNetParams *params, uint8_t *scratch)
{
  FujiNetParams local;
  void *buffer = params->buffer;
  uint16_t length = params->length;
  uint8_t ok;

  if (!STACK_IS_SAFE(&local) || !STACK_IS_SAFE(scratch))
    return 0;

  memcpy(&local, params, sizeof(local));
  local.buffer = scratch;

  if (func == FUJI_CALL_WRITE)
    memcpy(scratch, buffer, length);

  ok = enter(func, &local);

  if (ok && func == FUJI_CALL_READ)
    memcpy(buffer, scratch, local.length <= length ? local.length : length);

  unbounce(params, &local, buffer);
  return ok;
}

static uint8_t bounce_small(uint8_t func, FujiNetParams *params)
{
  uint8_t scratch[FUJI_UNAPI_SMALL_BOUNCE];

  return bounce_via(func, params, scratch);
}

static uint8_t bounce_large(uint8_t func, FujiNetParams *params)
{
  uint8_t scratch[FUJI_UNAPI_BOUNCE_SIZE];

  return bounce_via(func, params, scratch);
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

  /* A buffer the implementation cannot see is not something to paper over
     quietly: it reads its own ROM instead, and what comes back looks like a
     device that answered with nonsense rather than a caller that broke the
     rules. Bounce it if it fits, and fail if it does not. */
  if (params->length && params->buffer && IN_SWITCHED_PAGE(params->buffer)) {
    if (params->length <= FUJI_UNAPI_SMALL_BOUNCE)
      return bounce_small(func, params);
    if (params->length <= FUJI_UNAPI_BOUNCE_SIZE)
      return bounce_large(func, params);
    return 0;
  }

  return bounce_block(func, params);
}

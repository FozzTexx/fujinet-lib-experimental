#include "fujinet-bus-sms.h"
#include "fujinet-commands.h"

uint16_t fuji_bus_call_rlen;

void fn_regwr(uint8_t reg, uint8_t val)
{
  *(volatile uint8_t *) (FN_REGSEL + reg) = val;
}

/*
  The sequence number is the cart's own ACKSEQ + 1, never a variable of ours:
  the SMS1 Reset button restarts the program but not the cart, and a replayed
  number would be silently ignored with the old reply still in the window.
  The wait outlasts the cart's own budget (60 s for MOUNT_IMAGE), so a real
  timeout comes back as the cart's error code.
*/
uint8_t fn_commit(void)
{
  uint8_t want = (uint8_t) (FN_ACKSEQ + 1);
  uint16_t outer, inner;


  if (want == 0)
    want = 1;                   /* 0 means "never used" */
  fn_regwr(FNR_SEQ, want);

  for (outer = 0; outer < 4000u; outer++) {
    for (inner = 0; inner < 250u; inner++) {
      if (FN_ACKSEQ == want)
        return FN_ERRCODE;
    }
  }

  return FN_EWAIT;
}

/*
  Fixed four-aux signature, as adam, lynx, msdos and coleco: sccz80 lays a
  variadic callee's named parameters out as if only those had been pushed.
  fuji_field_numbytes() / fuji_field_numfields() give each parameter's size,
  always 1, 2 or 4, which is exactly what the TX stream takes.
*/
bool fuji_bus_call(uint8_t device, uint8_t fuji_cmd, uint8_t fields,
                   uint8_t aux1, uint8_t aux2, uint8_t aux3, uint8_t aux4,
                   const void *buf, size_t buf_length)
{
  uint8_t aux[4];
  uint8_t numbytes, numfields, size;
  uint8_t idx, field, i;
  uint16_t rlen;


  fuji_bus_call_rlen = 0;

  aux[0] = aux1;
  aux[1] = aux2;
  aux[2] = aux3;
  aux[3] = aux4;

  numbytes = fuji_field_numbytes(fields);
  numfields = fuji_field_numfields(fields);
  size = numfields ? (uint8_t) (numbytes / numfields) : 0;

  if ((fields & FUJI_FIELD_DATA)
      && (uint16_t) (numfields + numbytes) + buf_length > FN_TX_MAX)
    return false;

  fn_regwr(FNR_DATA_RST, 0);
  fn_regwr(FNR_DEVICE, device);
  fn_regwr(FNR_CMD, fuji_cmd);
  fn_regwr(FNR_NPARAM, 0);

  idx = 0;
  for (field = 0; field < numfields; field++) {
    FN_TXPAGE = size;
    for (i = 0; i < size; i++)
      FN_TXPAGE = aux[idx++];
  }
  if (numfields)
    fn_regwr(FNR_NPARAM, numfields);

  if (fields & FUJI_FIELD_DATA) {
    const uint8_t *data = (const uint8_t *) buf;
    size_t n = buf_length;

    while (n--)
      FN_TXPAGE = *data++;
  }

  if (fn_commit() != FN_OK)
    return false;

  if (FN_REPLYCMD != FUJICMD_ACK)
    return false;

  rlen = (uint16_t) FN_RXLEN_LO | ((uint16_t) FN_RXLEN_HI << 8);
  if (rlen > FN_REPLY_MAX)
    rlen = FN_REPLY_MAX;

  if (fields & FUJI_FIELD_REPLY) {
    uint8_t *reply = (uint8_t *) buf;
    /* through a local pointer: sccz80 mis-indexes the FN_REPLY cast macro */
    volatile uint8_t *src = FN_REPLY;
    uint16_t n;

    if (rlen > buf_length)
      rlen = buf_length;
    for (n = rlen; n; n--)
      *reply++ = *src++;
  }

  fuji_bus_call_rlen = rlen;

  return true;
}

/**
 * @brief Write string at s to DriveWire with length l
 * @param s pointer to string buffer
 * @param l length of string (0-65535 bytes)
 * @return error code
 */

#include <cmoc.h>
#include <coco.h>

#define COLUMNS 16

static void hexdump(uint8_t *buffer, int count)
{
  int outer, inner;
  uint8_t c;


  for (outer = 0; outer < count; outer += COLUMNS) {
    for (inner = 0; inner < COLUMNS; inner++) {
      if (inner + outer < count) {
	c = buffer[inner + outer];
	printf("%02x ", c);
      }
      else
	printf("   ");
    }
    printf(" |");
    for (inner = 0; inner < COLUMNS && inner + outer < count; inner++) {
      c = buffer[inner + outer];
      if (c >= ' ' && c <= 0x7f)
	printf("%c", c);
      else
	printf(".");
    }
    printf("|\n");
  }

  return;
}

byte dwwrite(byte *s, int l)
{
  printf("SENDING 0x%04x len=%d\n", s, l);
  hexdump(s, l);
    asm
    {
        pshs x,y
        ldx :s
        ldy :l
        jsr [0xD941]
        tfr cc,d
        puls y,x
    }
}

void pval(uint16_t val)
{
  printf("VAL=$%04x\n", val);
}

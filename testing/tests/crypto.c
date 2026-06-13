#include "broken.h"

#include "harness.h"
#include "globals.h"
#include "print_hex.h"
#include <fujinet-fuji.h>

#ifndef _CMOC_VERSION_
#include <stdio.h>
#include <string.h>
#endif /* _CMOC_VERSION_ */

#ifdef BUILD_C64
// Curse you PETSCII! You're making it hard to test!
#define BASE64_IN  {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20, 0x46, \
    0x75, 0x6A, 0x69, 0x4E, 0x65, 0x74, 0x21}
#else
#define BASE64_IN  "Hello, FujiNet!"
#endif
#define BASE64_OUT "SGVsbG8sIEZ1amlOZXQh\012"
#define MD5_OUT    "98d6da021c1a0b081b52de3b8207a823"
#define SHA1_OUT   "954382c7aa0a0d3252faf234ba4911f2aed22e39"
#define SHA256_OUT "3631e38122de15b2e5b4ae209ba1eb7c79b0e756" \
                   "fd9bc7149ce83da8aba6ddc6"
#define SHA512_OUT "8230a7a465c8ef21e6d89c423e5cb06f1f8b183b" \
                   "7928189736e00e81d269484d5a5d8a40723a157e" \
                   "f55d1992fd2929b213770eada5501434a592d75a" \
                   "d1f935fd"

#ifndef __CBM__
#define hexcmp strcasecmp
#define base64cmp strcmp
#else
static uint8_t petscii_to_ascii(uint8_t c)
{
  if (c >= 0x41 && c <= 0x5A)
    return c + 0x20; /* PETSCII lower -> ASCII lower */
  if (c >= 0x61 && c <= 0x7A)
    return c - 0x20; /* PETSCII upper1 -> ASCII upper */
  if (c >= 0xC1 && c <= 0xDA)
    return c - 0x80; /* PETSCII upper2 -> ASCII upper */
  return c;
}

/*
 * Compare a PETSCII string produced by the firmware against an ASCII hex
 * string literal in the source.  Case-insensitive on the ASCII side since
 * firmware may return either case.
 *
 * hex digits are 0-9 (identical in both encodings) and A-F/a-f (three
 * possible PETSCII representations, all handled by petscii_to_ascii).
 *
 * Returns 0 if equal, non-zero otherwise.
 */
int hexcmp(const char *petscii_result, const char *ascii_expected)
{
  uint8_t a;
  uint8_t b;


  while (*petscii_result || *ascii_expected) {
    a = petscii_to_ascii((uint8_t)*petscii_result);
    b = (uint8_t)*ascii_expected;

    /* fold both to lowercase for case-insensitive compare */
    if (a >= 0x41 && a <= 0x5A)
      a += 0x20;
    if (b >= 0x41 && b <= 0x5A)
      b += 0x20;

    if (a != b)
      return (int)a - (int)b;
    petscii_result++;
    ascii_expected++;
  }
  return 0;
}

/*
 * Compare a PETSCII string produced by the firmware against an ASCII base64
 * string literal in the source.  Case-sensitive since base64 is case-sensitive.
 *
 * Normalizes PETSCII alpha to ASCII alpha, preserving case.  Digits and the
 * base64 special characters '+', '/', '=' are identical in both encodings.
 *
 * Returns 0 if equal, non-zero otherwise.
 */
int base64cmp(const char *petscii_result, const char *ascii_expected)
{
  uint8_t a;
  uint8_t b;


  while (*petscii_result || *ascii_expected) {
    a = petscii_to_ascii((uint8_t)*petscii_result);
    b = (uint8_t)*ascii_expected;

    if (a != b)
      return (int)a - (int)b;
    petscii_result++;
    ascii_expected++;
  }
  return 0;
}
#endif

void test_fuji_base64(void)
{
  char input[] = BASE64_IN;
  uint16_t in_len;
  bool ok;
  unsigned long enc_len;
  unsigned long dec_len;

  SECTION("fuji base64 encode/decode");

    /*
     * The firmware has no way to clear base64 state other than calling
     * output. Drain both encode and decode pipelines before starting so
     * we know we're in a clean state regardless of what ran before.
     */
#ifdef FN_BROKEN_fuji_base64_encode_length
  SKIP(fuji_base64_encode_length);
#endif
  enc_len = 0;
  ok = fuji_base64_encode_length(&enc_len);
#ifdef FN_BROKEN_fuji_base64_encode_output
  SKIP(fuji_base64_encode_output);
#endif
  if (ok && enc_len) {
    printf("Clearing encode %ld\n", enc_len);
    for (; enc_len; enc_len -= in_len) {
      in_len = enc_len > sizeof(g.net) ? sizeof(g.net) : (uint16_t) enc_len;
      fuji_base64_encode_output((char *) g.net, in_len);
    }
  }

#ifdef FN_BROKEN_fuji_base64_decode_length
  SKIP(fuji_base64_decode_length);
#endif
  dec_len = 0;
  ok = fuji_base64_decode_length(&dec_len);
#ifdef FN_BROKEN_fuji_base64_decode_output
  SKIP(fuji_base64_decode_output);
#endif
  if (ok && dec_len) {
    printf("Clearing decode %ld\n", dec_len);
    for (; dec_len; enc_len -= in_len) {
      in_len = dec_len > sizeof(g.net) ? sizeof(g.net) : (uint16_t) dec_len;
      fuji_base64_decode_output((char *) g.net, in_len);
    }
  }

  in_len = (uint16_t)strlen(input);

#ifdef FN_BROKEN_fuji_base64_encode_input
  SKIP(fuji_base64_encode_input);
#endif
  ok = fuji_base64_encode_input(input, in_len);
  TEST("fuji_base64_encode_input succeeds", ok);

#ifdef FN_BROKEN_fuji_base64_encode_compute
  SKIP(fuji_base64_encode_compute);
#endif
  ok = fuji_base64_encode_compute();
  TEST("fuji_base64_encode_compute succeeds", ok);

#ifdef FN_BROKEN_fuji_base64_encode_length
  SKIP(fuji_base64_encode_length);
#endif
  enc_len = 0;
  ok = fuji_base64_encode_length(&enc_len);
  TEST("fuji_base64_encode_length succeeds", ok);
  TEST("Encoded length is positive", enc_len > 0);
  TEST("Encoded length fits in g.b64.enc", enc_len < sizeof(g.b64.enc));

#ifdef FN_BROKEN_fuji_base64_encode_output
  SKIP(fuji_base64_encode_output);
#endif
  ok = fuji_base64_encode_output(g.b64.enc, (uint16_t)enc_len);
  TEST("fuji_base64_encode_output succeeds", ok);
  g.b64.enc[(uint16_t) enc_len] = '\0';
  printf("  Base64 encoded: %s\n", g.b64.enc);
  TEST("Base64 output matches expected value", base64cmp(g.b64.enc, BASE64_OUT) == 0);

#ifdef FN_BROKEN_fuji_base64_decode_input
  SKIP(fuji_base64_decode_input);
#endif
  ok = fuji_base64_decode_input(g.b64.enc, (uint16_t)enc_len);
  TEST("fuji_base64_decode_input succeeds", ok);

#ifdef FN_BROKEN_fuji_base64_decode_compute
  SKIP(fuji_base64_decode_compute);
#endif
  ok = fuji_base64_decode_compute();
  TEST("fuji_base64_decode_compute succeeds", ok);

#ifdef FN_BROKEN_fuji_base64_decode_length
  SKIP(fuji_base64_decode_length);
#endif
  dec_len = 0;
  ok = fuji_base64_decode_length(&dec_len);
  TEST("fuji_base64_decode_length succeeds", ok);
  TEST("Decoded length fits in g.b64.dec", dec_len < sizeof(g.b64.dec));

#ifdef FN_BROKEN_fuji_base64_decode_output
  SKIP(fuji_base64_decode_output);
#endif
  ok = fuji_base64_decode_output(g.b64.dec, (uint16_t)dec_len);
  TEST("fuji_base64_decode_output succeeds", ok);
  g.b64.dec[(uint16_t) dec_len] = '\0';
  printf("  Base64 decoded: %s\n", g.b64.dec);
  TEST("Decoded output matches original input", strcmp(g.b64.dec, input) == 0);

  END_OF_TEST();
}

void test_fuji_guid(void)
{
  char guid1[MAX_GUID_LEN];
  char guid2[MAX_GUID_LEN];
  bool ok1;
  bool ok2;

  SECTION("fuji_generate_guid");

#ifdef FN_BROKEN_fuji_generate_guid
  SKIP(fuji_generate_guid);
#else
  memset(guid1, 0, sizeof(guid1));
  memset(guid2, 0, sizeof(guid2));

  ok1 = fuji_generate_guid(guid1);
  ok2 = fuji_generate_guid(guid2);
  TEST("First fuji_generate_guid succeeds", ok1);
  TEST("Second fuji_generate_guid succeeds", ok2);
  TEST("GUID1 length is 36", strlen(guid1) == 36);
  TEST("GUID2 length is 36", strlen(guid2) == 36);
  TEST("GUIDs are unique", strcmp(guid1, guid2) != 0);
  printf("  GUID1: %s\n  GUID2: %s\n", guid1, guid2);
#endif

  END_OF_TEST();
}

void test_fuji_hashing(void)
{
  uint8_t input[] = BASE64_IN;
  uint16_t in_len;
  uint16_t sz;
  bool ok;

  SECTION("fuji hashing (fuji_hash_data single-shot)");

  in_len = (uint16_t)strlen((char *)input);

#ifdef FN_BROKEN_fuji_hash_size
  SKIP(fuji_hash_size);
#endif
#ifdef FN_BROKEN_fuji_hash_data
  SKIP(fuji_hash_data);
#else

  sz = fuji_hash_size(MD5, true);
  TEST("fuji_hash_size(MD5, hex) returns 32", sz == 32);
  memset(g.hash.md5, 0, sizeof(g.hash.md5));
  ok = fuji_hash_data(MD5, input, in_len, true, g.hash.md5);
  TEST("fuji_hash_data MD5 succeeds", ok);
  g.hash.md5[sz] = '\0';
  printf("  MD5 hex: %s\n", g.hash.md5);
  TEST("MD5 output length is 32 hex chars", strlen((char *)g.hash.md5) == 32);
  TEST("MD5 output matches expected value", hexcmp((char *)g.hash.md5, MD5_OUT) == 0);

  sz = fuji_hash_size(SHA1, true);
  TEST("fuji_hash_size(SHA1, hex) returns 40", sz == 40);
  ok = fuji_hash_data(SHA1, input, in_len, true, g.hash.sha1);
  TEST("fuji_hash_data SHA1 succeeds", ok);
  g.hash.sha1[sz] = '\0';
  printf("  SHA1 hex: %s\n", g.hash.sha1);
  TEST("SHA1 output length is 40 hex chars", strlen((char *)g.hash.sha1) == 40);
  TEST("SHA1 output matches expected value", hexcmp((char *)g.hash.sha1, SHA1_OUT) == 0);

  sz = fuji_hash_size(SHA256, true);
  TEST("fuji_hash_size(SHA256, hex) returns 64", sz == 64);
  ok = fuji_hash_data(SHA256, input, in_len, true, g.hash.sha256);
  TEST("fuji_hash_data SHA256 succeeds", ok);
  g.hash.sha256[sz] = '\0';
  printf("  SHA256 hex: %s\n", g.hash.sha256);
  TEST("SHA256 output length is 64 hex chars", strlen((char *)g.hash.sha256) == 64);
  TEST("SHA256 output matches expected value", hexcmp((char *)g.hash.sha256, SHA256_OUT) == 0);

  sz = fuji_hash_size(SHA512, true);
  TEST("fuji_hash_size(SHA512, hex) returns 128", sz == 128);
  ok = fuji_hash_data(SHA512, input, in_len, true, g.hash.sha512);
  TEST("fuji_hash_data SHA512 succeeds", ok);
  g.hash.sha512[sz] = '\0';
  printf("  SHA512 hex: %s\n", g.hash.sha512);
  TEST("SHA512 output length is 128 hex chars", strlen((char *)g.hash.sha512) == 128);
  TEST("SHA512 output matches expected value", hexcmp((char *)g.hash.sha512, SHA512_OUT) == 0);

  SECTION("fuji hashing (incremental interface)");
#endif

#ifdef FN_BROKEN_fuji_hash_clear
  SKIP(fuji_hash_clear);
#else
  ok = fuji_hash_clear();
  TEST("fuji_hash_clear succeeds", ok);
#endif

#ifdef FN_BROKEN_fuji_hash_add
  SKIP(fuji_hash_add);
#else
  ok = fuji_hash_add(input, in_len);
  TEST("fuji_hash_add succeeds", ok);
#endif

#ifdef FN_BROKEN_fuji_hash_calculate
  SKIP(fuji_hash_calculate);
#else
  memset(g.hash_verify.inc1, 0, sizeof(g.hash_verify.inc1));
  ok = fuji_hash_calculate(SHA256, true, false, g.hash_verify.inc1);
  TEST("fuji_hash_calculate(SHA256, keep) succeeds", ok);
  TEST("Incremental SHA256 length is 64 hex chars", strlen((char *)g.hash_verify.inc1) == 64);

  memset(g.hash_verify.inc2, 0, sizeof(g.hash_verify.inc2));
  ok = fuji_hash_calculate(SHA256, true, true, g.hash_verify.inc2);
  TEST("fuji_hash_calculate(SHA256, discard) succeeds", ok);
  TEST("Second incremental calculation matches first",
       memcmp(g.hash_verify.inc1, g.hash_verify.inc2, 64) == 0);
  TEST("Incremental SHA256 matches single-shot result",
       memcmp(g.hash_verify.inc1, g.hash.sha256, 64) == 0);
#endif

  END_OF_TEST();
}

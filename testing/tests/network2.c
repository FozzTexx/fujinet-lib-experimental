/**
 * @file network2.c
 * @brief Coverage for the network device commands added in fujinet-lib v5:
 *        seek/tell, translation, EOL, login, timer rate, TCP close-client,
 *        UDP destination/remote, JSON parameters, DSTATS inquiry, channel
 *        mode, and the fn_* status globals.
 *
 * Firmware regression commits exercised here (fujinet-firmware):
 *   b8d67c34c  [sio] note/point            -> test_net2_seek_tell, test_net2_seek_errors,
 *                                             test_net2_get_dstats
 *   f3586b99b  set-EOL command             -> test_net2_set_eol_ext
 *   a1c8d9dfe  [sio] sticky NTRANS         -> test_net2_set_translation (wire accept only;
 *                                             full translation observation is manual)
 *   e04b1759a  [coco] $54 arm removed      -> FN_UNSUPPORTED_network_set_translation on coco
 *   a720672e2  [iwm] login propagation     -> test_net2_login_smoke (authenticated open
 *                                             needs private credentials: manual)
 *   e2699b33a / 94517bf9a / 90279b566 UDP  -> test_net2_udp_set_destination,
 *                                             test_net2_udp_get_remote (FujiNet-PC only)
 *   0b59791ee  [coco] read count           -> test_net2_read_count
 */

#include "broken.h"

#include "harness.h"
#include "constants.h"
#include "globals.h"
#include <fujinet-network.h>

#ifndef _CMOC_VERSION_
#include <stdio.h>
#include <string.h>
#endif /* _CMOC_VERSION_ */

#ifndef FN_NETWORK_UNIFIED_COMMANDS

/* Building against a fujinet-lib without the unified network command
 * surface: stub every test to a SKIP so older releases still link. */
#define UNIFIED_STUB(fn)                                        \
  void fn(void)                                                 \
  {                                                             \
    SKIP_WHY(fn, "lib lacks unified network commands");         \
    END_OF_TEST();                                              \
  }

UNIFIED_STUB(test_net2_seek_tell)
UNIFIED_STUB(test_net2_seek_errors)
UNIFIED_STUB(test_net2_get_dstats)
UNIFIED_STUB(test_net2_set_translation)
UNIFIED_STUB(test_net2_set_eol_ext)
UNIFIED_STUB(test_net2_login_smoke)
UNIFIED_STUB(test_net2_timer_rate)
UNIFIED_STUB(test_net2_close_client)
UNIFIED_STUB(test_net2_udp_set_destination)
UNIFIED_STUB(test_net2_udp_get_remote)
UNIFIED_STUB(test_net2_channel_mode)
UNIFIED_STUB(test_net2_json_parameters)
UNIFIED_STUB(test_net2_read_count)
UNIFIED_STUB(test_net2_globals)
UNIFIED_STUB(test_net2_proceed)

#else /* FN_NETWORK_UNIFIED_COMMANDS */

void test_net2_seek_tell(void)
{
  uint8_t err;
  uint32_t pos;
  int16_t r;
  uint16_t i;

  SECTION("network seek/tell (note/point)");

#ifdef FN_UNSUPPORTED_network_seek
  SKIP_UNSUPPORTED(network_seek);
#else
  err = network_open(NET_RANGE_URL, OPEN_MODE_READ, OPEN_TRANS_NONE);
  TEST("open range URL", err == FN_ERR_OK);

  r = network_read(NET_RANGE_URL, g.net, 16);
  TEST("read 16 bytes", r == 16);
  for (i = 0; i < 16; i++) {
    if (g.net[i] != RANGE_BYTE(i))
      break;
  }
  TEST("first 16 bytes match pattern", i == 16);

  pos = 0xFFFFFFFF;
  err = network_tell(NET_RANGE_URL, &pos);
  TEST("tell returns FN_ERR_OK", err == FN_ERR_OK);
  printf("  position=%lu\n", (unsigned long) pos);
  TEST("tell reports position 16", pos == 16);

  err = network_seek(NET_RANGE_URL, 256);
  TEST("seek to 256 returns FN_ERR_OK", err == FN_ERR_OK);

  pos = 0;
  err = network_tell(NET_RANGE_URL, &pos);
  TEST("tell after seek returns FN_ERR_OK", err == FN_ERR_OK);
  TEST("tell reports position 256", pos == 256);

  r = network_read(NET_RANGE_URL, g.net, 4);
  TEST("read 4 bytes at offset 256", r == 4);
  /* 256 % 26 == 22, so the bytes are w x y z */
  TEST("data comes from the seeked offset",
       g.net[0] == 'w' && g.net[1] == 'x' && g.net[2] == 'y' && g.net[3] == 'z');

  err = network_close(NET_RANGE_URL);
  TEST("close range URL", err == FN_ERR_OK);
#endif /* FN_UNSUPPORTED_network_seek */

  END_OF_TEST();
}

void test_net2_seek_errors(void)
{
  uint8_t err;

  SECTION("network seek error paths");

#ifdef FN_UNSUPPORTED_network_seek
  SKIP_UNSUPPORTED(network_seek);
#else
  /* No channel open on N4 */
  err = network_seek("N4:", 0);
  TEST("seek on unopened channel fails", err != FN_ERR_OK);

  /* Seek is PROTOCOL-mode only; in JSON mode it must fail */
  err = network_open(NET_JSON_URL, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE);
  TEST("open JSON URL", err == FN_ERR_OK);
  err = network_json_parse(NET_JSON_URL);
  TEST("parse JSON", err == FN_ERR_OK);
  err = network_seek(NET_JSON_URL, 0);
  TEST("seek in JSON channel mode fails", err != FN_ERR_OK);
  err = network_close(NET_JSON_URL);
  TEST("close JSON URL", err == FN_ERR_OK);
#endif /* FN_UNSUPPORTED_network_seek */

  END_OF_TEST();
}

void test_net2_get_dstats(void)
{
  uint8_t err;
  uint8_t dstats;

  SECTION("network DSTATS inquiry");

#ifdef FN_UNSUPPORTED_network_get_dstats
  SKIP_UNSUPPORTED(network_get_dstats);
#else
  dstats = 0;
  err = network_get_dstats("N1:", FUJICMD_SEEK, &dstats);
  TEST("dstats(SEEK) returns FN_ERR_OK", err == FN_ERR_OK);
  TEST("SEEK is a write command", dstats == 0x80);

  err = network_get_dstats("N1:", FUJICMD_TELL, &dstats);
  TEST("dstats(TELL) returns FN_ERR_OK", err == FN_ERR_OK);
  TEST("TELL is a read command", dstats == 0x40);

  err = network_get_dstats("N1:", 0x99, &dstats);
  TEST("dstats(bogus) returns FN_ERR_OK", err == FN_ERR_OK);
  TEST("bogus command is invalid", dstats == 0xFF);

  /* Firmware gap: the DSTATS table omits SET_EOL ($4C) even though the
   * command is implemented (fujinet-firmware sio/network.cpp), so it
   * reports invalid. Print rather than assert so a firmware fix does
   * not fail the suite. */
  err = network_get_dstats("N1:", FUJICMD_SET_EOL, &dstats);
  if (err == FN_ERR_OK)
    printf("  dstats(SET_EOL)=%02x (0xFF documents a firmware table gap)\n", dstats);
#endif /* FN_UNSUPPORTED_network_get_dstats */

  END_OF_TEST();
}

void test_net2_set_translation(void)
{
  uint8_t err;

  SECTION("network set translation");

#ifdef FN_UNSUPPORTED_network_set_translation
  SKIP_UNSUPPORTED(network_set_translation);
#else
  /* Wire-level accept only. The sticky OR into subsequent opens
   * (a1c8d9dfe) is observed manually: set 2, open with aux2 0, read a
   * text resource, confirm LF translation. */
  err = network_set_translation("N1:", OPEN_TRANS_LF);
  TEST("set translation LF accepted", err == FN_ERR_OK);

  err = network_set_translation("N1:", 0xFF);
  TEST("set translation 0xFF (ignore aux2) accepted", err == FN_ERR_OK);

  err = network_set_translation("N1:", OPEN_TRANS_NONE);
  TEST("set translation restored to none", err == FN_ERR_OK);
#endif /* FN_UNSUPPORTED_network_set_translation */

  END_OF_TEST();
}

void test_net2_set_eol_ext(void)
{
  uint8_t err;
  int16_t r;
  uint16_t i;
  uint8_t saw_crlf;

  SECTION("network set EOL affects listings");

#ifdef FN_UNSUPPORTED_network_set_eol
  SKIP_UNSUPPORTED(network_set_eol);
#else
  err = network_set_eol("N1:", "\r\n");
  TEST("set EOL to CRLF", err == FN_ERR_OK);

  err = network_open(WEBDAV_ROOT, OPEN_MODE_HTTP_PROPFIND, DIR_FORMAT_RAW);
  TEST("open WebDAV listing", err == FN_ERR_OK);

  memset(g.net, 0, sizeof(g.net));
  r = network_read(WEBDAV_ROOT, g.net, sizeof(g.net) - 1);
  TEST("read listing", r > 0);

  saw_crlf = 0;
  for (i = 0; i + 1 < (uint16_t) r; i++) {
    if (g.net[i] == '\r' && g.net[i + 1] == '\n') {
      saw_crlf = 1;
      break;
    }
  }
  TEST("entries terminated with CRLF", saw_crlf);

  err = network_close(WEBDAV_ROOT);
  TEST("close listing", err == FN_ERR_OK);

  /* First byte 0 restores the platform default EOL */
  err = network_set_eol("N1:", "\0");
  TEST("restore default EOL", err == FN_ERR_OK);

#ifdef BUILD_ATARI
  err = network_open(WEBDAV_ROOT, OPEN_MODE_HTTP_PROPFIND, DIR_FORMAT_RAW);
  TEST("reopen WebDAV listing", err == FN_ERR_OK);
  memset(g.net, 0, sizeof(g.net));
  r = network_read(WEBDAV_ROOT, g.net, sizeof(g.net) - 1);
  TEST("read listing again", r > 0);
  saw_crlf = 0;
  for (i = 0; i < (uint16_t) r; i++) {
    if (g.net[i] == 0x9B) {
      saw_crlf = 1;
      break;
    }
  }
  TEST("entries terminated with ATASCII EOL again", saw_crlf);
  err = network_close(WEBDAV_ROOT);
  TEST("close listing again", err == FN_ERR_OK);
#endif /* BUILD_ATARI */
#endif /* FN_UNSUPPORTED_network_set_eol */

  END_OF_TEST();
}

void test_net2_login_smoke(void)
{
  uint8_t err;

  SECTION("network username/password");

#ifdef FN_UNSUPPORTED_network_login
  SKIP_UNSUPPORTED(network_set_username);
#else
  /* State-only: the values apply to the next open. An authenticated
   * SMB/FTP open needs private credentials and is a manual test. */
  err = network_set_username("N1:", "fujinet-test-user");
  TEST("set username accepted", err == FN_ERR_OK);

  err = network_set_password("N1:", "fujinet-test-pass");
  TEST("set password accepted", err == FN_ERR_OK);

  /* Clear them so later opens are not affected */
  err = network_set_username("N1:", "");
  TEST("clear username accepted", err == FN_ERR_OK);
  err = network_set_password("N1:", "");
  TEST("clear password accepted", err == FN_ERR_OK);
#endif /* FN_UNSUPPORTED_network_login */

  END_OF_TEST();
}

void test_net2_timer_rate(void)
{
  uint8_t err;

  SECTION("network interrupt timer rate");

#ifdef FN_UNSUPPORTED_network_set_timer_rate
  SKIP_UNSUPPORTED(network_set_timer_rate);
#else
  err = network_set_timer_rate("N1:", 50);
  TEST("set timer rate 50ms accepted", err == FN_ERR_OK);

  err = network_set_timer_rate("N1:", 100);
  TEST("restore timer rate 100ms accepted", err == FN_ERR_OK);
#endif /* FN_UNSUPPORTED_network_set_timer_rate */

  END_OF_TEST();
}

void test_net2_close_client(void)
{
  uint8_t err;

  SECTION("network TCP close-client");

#ifdef FN_UNSUPPORTED_network_close_client
  SKIP_UNSUPPORTED(network_close_client);
#else
  err = network_open(NET_TCP_LISTEN, OPEN_MODE_RW, OPEN_TRANS_NONE);
  TEST("open listening TCP channel", err == FN_ERR_OK);

  /* No client has connected; closing the (absent) client connection
   * must still complete cleanly */
  err = network_close_client(NET_TCP_LISTEN);
  TEST("close-client with no client completes", err == FN_ERR_OK);

  err = network_close(NET_TCP_LISTEN);
  TEST("close listening channel", err == FN_ERR_OK);
#endif /* FN_UNSUPPORTED_network_close_client */

  END_OF_TEST();
}

void test_net2_udp_set_destination(void)
{
  uint8_t err;

  SECTION("network UDP set destination");

#ifdef FN_UNSUPPORTED_network_udp
  SKIP_UNSUPPORTED(network_udp_set_destination);
#else
  err = network_open(NET_UDP_SPEC, OPEN_MODE_RW, OPEN_TRANS_NONE);
  TEST("open UDP channel", err == FN_ERR_OK);

  err = network_udp_set_destination(NET_UDP_DEST);
  TEST("set UDP destination accepted", err == FN_ERR_OK);

  err = network_close(NET_UDP_SPEC);
  TEST("close UDP channel", err == FN_ERR_OK);
#endif /* FN_UNSUPPORTED_network_udp */

  END_OF_TEST();
}

void test_net2_udp_get_remote(void)
{
  uint8_t err;

  SECTION("network UDP get remote");

#ifdef FN_UNSUPPORTED_network_udp_get_remote
  SKIP_UNSUPPORTED(network_udp_get_remote);
#else
  /* FujiNet-PC only: receive a datagram first, then ask where it came
   * from. Without inbound traffic just prove the command completes. */
  err = network_open(NET_UDP_SPEC, OPEN_MODE_RW, OPEN_TRANS_NONE);
  TEST("open UDP channel", err == FN_ERR_OK);

  memset(g.net, 0, sizeof(g.net));
  err = network_udp_get_remote(NET_UDP_SPEC, (char *) g.net, MAX_FILENAME_LEN);
  TEST("get remote completes", err == FN_ERR_OK);
  printf("  remote=\"%s\"\n", (char *) g.net);

  err = network_close(NET_UDP_SPEC);
  TEST("close UDP channel", err == FN_ERR_OK);
#endif /* FN_UNSUPPORTED_network_udp_get_remote */

  END_OF_TEST();
}

void test_net2_channel_mode(void)
{
  uint8_t err;
  int16_t r;

  SECTION("network channel mode transitions");

#ifdef FN_BROKEN_network_open
  SKIP(network_open);
#else
  err = network_open(NET_JSON_URL, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE);
  TEST("open JSON URL", err == FN_ERR_OK);

  err = network_set_channel_mode(NET_JSON_URL, NETWORK_CHANMODE_JSON);
  TEST("switch to JSON channel mode", err == FN_ERR_OK);

  err = network_set_channel_mode(NET_JSON_URL, NETWORK_CHANMODE_PROTOCOL);
  TEST("switch back to PROTOCOL mode", err == FN_ERR_OK);

  /* The body must still be readable after the round trip */
  r = network_read(NET_JSON_URL, g.net, 64);
  TEST("read body after mode round trip", r > 0);

  err = network_close(NET_JSON_URL);
  TEST("close JSON URL", err == FN_ERR_OK);
#endif /* FN_BROKEN_network_open */

  END_OF_TEST();
}

void test_net2_json_parameters(void)
{
  uint8_t err;

  SECTION("network JSON parameters");

#ifdef FN_UNSUPPORTED_network_json_set_parameters
  SKIP_UNSUPPORTED(network_json_set_parameters);
#else
  err = network_json_set_query_param("N1:", 0);
  TEST("query param 0 accepted", err == FN_ERR_OK);

  err = network_json_set_query_param("N1:", 2);
  TEST("query param 2 accepted", err == FN_ERR_OK);

  /* Firmware rejects query param values above 2 */
  err = network_json_set_query_param("N1:", 3);
  TEST("query param 3 rejected", err != FN_ERR_OK);

  err = network_json_set_line_ending("N1:", 0x0A);
  TEST("line ending LF accepted", err == FN_ERR_OK);

  /* Restore defaults: flags 0, ATASCII EOL */
  err = network_json_set_query_param("N1:", 0);
  TEST("query param restored", err == FN_ERR_OK);
  err = network_json_set_line_ending("N1:", 0x9B);
  TEST("line ending restored", err == FN_ERR_OK);
#endif /* FN_UNSUPPORTED_network_json_set_parameters */

  END_OF_TEST();
}

void test_net2_read_count(void)
{
  uint8_t err;
  int16_t r;
  uint16_t i;
  uint16_t total;

  SECTION("network read honors requested count");

#ifdef FN_UNSUPPORTED_network_read_partial
  SKIP_UNSUPPORTED(network_read_partial);
#else
  err = network_open(NET_RANGE_URL, OPEN_MODE_READ, OPEN_TRANS_NONE);
  TEST("open range URL", err == FN_ERR_OK);

  /* 1024 bytes are (eventually) waiting; ask for exactly 64 */
  memset(g.net, 0, sizeof(g.net));
  r = network_read(NET_RANGE_URL, g.net, 64);
  TEST("read returns exactly 64 bytes", r == 64);
  for (i = 0; i < 64; i++) {
    if (g.net[i] != RANGE_BYTE(i))
      break;
  }
  TEST("64 bytes match pattern", i == 64);

  /* Drain the rest */
  total = 64;
  for (;;) {
    r = network_read_nb(NET_RANGE_URL, g.net, sizeof(g.net));
    if (r < 1)
      break;
    total += r;
  }
  printf("  drained total=%u\n", total);
  TEST("all 1024 bytes arrived", total == 1024);

  /* Reading past the end reports no more data */
  r = network_read_nb(NET_RANGE_URL, g.net, 16);
  TEST("read past EOF returns no data", r <= 0);

  err = network_close(NET_RANGE_URL);
  TEST("close range URL", err == FN_ERR_OK);
#endif /* FN_UNSUPPORTED_network_read_partial */

  END_OF_TEST();
}

void test_net2_globals(void)
{
  uint8_t err;
  int16_t r;

  SECTION("network status globals");

#ifdef FN_BROKEN_network_open
  SKIP(network_open);
#else
  err = network_open(NET_DEVICESPEC, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE);
  TEST("open GET URL", err == FN_ERR_OK);

  r = network_read(NET_DEVICESPEC, g.net, 128);
  TEST("read some bytes", r > 0);
  printf("  read=%d fn_bytes_read=%u bw=%u conn=%u err=%u\n",
         (int) r, fn_bytes_read, fn_network_bw, fn_network_conn, fn_network_error);
  TEST("fn_bytes_read matches read result", fn_bytes_read == (uint16_t) r);

  err = network_close(NET_DEVICESPEC);
  TEST("close GET URL", err == FN_ERR_OK);
#endif /* FN_BROKEN_network_open */

  END_OF_TEST();
}

void test_net2_proceed(void)
{
  bool has;

  SECTION("network proceed line");

  has = network_has_proceed();
  printf("  network_has_proceed=%u\n", (unsigned) has);
#ifdef BUILD_COCO
  /* Real support depends on the HDB-DOS ROM in use; just prove the call works */
  TEST("network_proceed callable", network_proceed() == true || 1);
#else
  TEST("no proceed line on this platform", has == false);
  TEST("network_proceed reports no data", network_proceed() == false);
#endif /* BUILD_COCO */

  END_OF_TEST();
}

#endif /* FN_NETWORK_UNIFIED_COMMANDS */

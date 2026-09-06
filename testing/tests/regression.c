/**
 * @file regression.c
 * @brief Regression tests for client-observable network device changes in
 *        fujinet-firmware v1.6.1..master (lib/device/<bus>/network.*).
 *
 * Commit -> test map (this file):
 *   013e97979 [coco] process_fs fetches params   -> test_regr_fs_no_open
 *   a142b8dff [rs232] N: filesystem commands     -> test_regr_fs_no_open
 *   d42360782 [coco] close with no protocol      -> test_regr_close_never_opened
 *   d08d38c91 chdir payload (sio) / getcwd (coco)-> test_regr_chdir_pwd_roundtrip
 *   4e81a9815 [iwm] open error preserved         -> test_regr_open_error_status
 *   a6b481cbd [iwm] missing bus replies          -> test_regr_bad_open_then_write,
 *                                                   test_regr_apple2_set_channel
 *   8a2345741 [adamnet] NULL deref on open/write -> test_regr_bad_open_then_write
 *   31196e482 long dir entry width               -> test_regr_dir_listing_width
 *   232612375 [rs232] stale devicespec on open   -> test_regr_stale_devicespec
 *   819f1e938 [adam] error visibility ($45)      -> test_regr_adam_get_error
 *   (comlynx)  $80/$81 parse/query aliases       -> test_regr_lynx_alt_parse_query
 *
 * Covered by other test binaries:
 *   7b95bf01b status after close                 -> network.c test_network_status_after_close
 *   9d3a081bd no-N-prefix URL                    -> network.c test_network_open_no_n_prefix
 *   37d935d20 [iwm] HTTP POST                    -> network.c test_network_http_post
 *   819f1e938 [adam] HTTP channel mode ACK       -> network.c test_network_http_channel_mode
 *   b8d67c34c [sio] note/point                   -> network2.c test_net2_seek_tell
 *   f3586b99b set-EOL                            -> network2.c test_net2_set_eol_ext
 *   9bf45ddae SGML channel mode                  -> sgml.c
 *   83bff9a79/c026d9a12 calendar                 -> calmail.c
 *   7ad7113d3 mail                               -> calmail.c (credential gated)
 *   0b59791ee [coco] read count                  -> network2.c test_net2_read_count
 *   e2699b33a/94517bf9a/90279b566 UDP get-remote -> network2.c (FujiNet-PC only)
 *
 * Manual (not automatable from this suite):
 *   02d041ed0 High Speed SIO boot: enable HSIO in CONFIG, run the full
 *             fnettest/fnet2test pass at the elevated divisor.
 *   459fb6b9b SmartPort OPEN/$06 CLOSE/$07 verbs (Apple /// path): the
 *             lib never issues them; needs a dedicated SmartPort tool.
 *   5b9047177 [adam] 1024-byte payload boundary: needs a >512-byte
 *             single write, larger than the lib's AdamNet packet buffer.
 *   a720672e2 [iwm] login propagation: needs private SMB/FTP credentials.
 *   a1c8d9dfe [sio] sticky NTRANS full observation: set 2, open aux2=0,
 *             read a LF text file, confirm EOL conversion on screen.
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

#define REGR_DIR_NAME "fnregr-" PLATFORM
#define REGR_DIR_URL  "N1:HTTPS://" WEBDAV_HOST "/" REGR_DIR_NAME
#define REGR_DIR_LIST "N1:HTTPS://" WEBDAV_HOST "/" REGR_DIR_NAME "/"

#define REGR_LONG_URL  "N1:https://httpbin.dev/range/26"
#define REGR_SHORT_URL "N1:https://httpbin.dev/get"

#define TNFS_ROOT "N1:TNFS://" TNFS_COPY_SOURCE_HOST "/"
#define TNFS_MISSING "N1:TNFS://" TNFS_COPY_SOURCE_HOST "/no-such-file-xyzzy.bin"

/* Portable substring search */
static uint8_t t_contains(const char *hay, const char *needle)
{
  uint16_t i, j;

  for (i = 0; hay[i]; i++) {
    for (j = 0; needle[j] && hay[i + j] == needle[j]; j++)
      ;
    if (!needle[j])
      return 1;
  }
  return 0;
}

/* NOTE: run this first in the binary. The fs commands must instantiate
 * their own protocol from the command's devicespec; before 013e97979
 * (coco) and a142b8dff (rs232) they only worked, if at all, by riding
 * on a protocol left over from an earlier open. */
void test_regr_fs_no_open(void)
{
  uint8_t err;

  SECTION("fs commands with no prior open");

#ifdef FN_BROKEN_network_open
  SKIP(network_fs_mkdir);
#else
  err = network_fs_mkdir(REGR_DIR_URL);
  TEST("mkdir with no open channel", err == FN_ERR_OK);

  /* The directory must really exist on the server */
  err = network_open(REGR_DIR_LIST, OPEN_MODE_HTTP_PROPFIND, DIR_FORMAT_RAW);
  TEST("new directory is listable", err == FN_ERR_OK);
  network_close(REGR_DIR_LIST);

  err = network_fs_rmdir(REGR_DIR_URL);
  TEST("rmdir with no open channel", err == FN_ERR_OK);
#endif /* FN_BROKEN_network_open */

  END_OF_TEST();
}

void test_regr_close_never_opened(void)
{
  uint8_t err;

  SECTION("close of a never-opened unit");

  /* d42360782: close with no protocol must still complete the bus
   * transaction instead of leaving the host hanging */
  err = network_close("N3:");
  TEST("close never-opened N3 completes", err == FN_ERR_OK);

  END_OF_TEST();
}

void test_regr_chdir_pwd_roundtrip(void)
{
  uint8_t err;

  SECTION("chdir / getcwd round trip");

#ifdef FN_BROKEN_network_fs_pwd
  SKIP(network_fs_pwd);
#else
  /* d08d38c91: sio chdir now receives its payload (WILL_GET) and coco
   * getcwd completes its transaction */
  err = network_fs_cd("N2:TNFS://" TNFS_COPY_SOURCE_HOST "/");
  TEST("chdir to TNFS root", err == FN_ERR_OK);

  memset(g.fs.base, 0, sizeof(g.fs.base));
  err = network_fs_pwd("N2:", g.fs.base);
  TEST("getcwd completes", err == FN_ERR_OK);
  printf("  pwd=\"%s\"\n", g.fs.base);
  TEST("prefix round-trips", t_contains(g.fs.base, TNFS_COPY_SOURCE_HOST));

  /* Clear the prefix again */
  err = network_fs_cd("N2:");
  TEST("prefix cleared", err == FN_ERR_OK);
#endif /* FN_BROKEN_network_fs_pwd */

  END_OF_TEST();
}

void test_regr_open_error_status(void)
{
  uint8_t err;
  uint16_t bw;
  uint8_t conn, nerr;

  SECTION("failed open reports the real error");

  err = network_open(TNFS_MISSING, OPEN_MODE_READ, OPEN_TRANS_NONE);
#ifdef BUILD_APPLE2
  /* IWM ACKs the open at bus level; 4e81a9815 preserves the real error
   * for the following status instead of reporting INVALID_COMMAND */
  bw = 0; conn = 0; nerr = 0;
  network_status(TNFS_MISSING, &bw, &conn, &nerr);
  printf("  status err=%u\n", nerr);
  TEST("status carries FILE_NOT_FOUND", nerr == 170);
#else
  TEST("open of missing file fails", err != FN_ERR_OK);
  bw = 0; conn = 0; nerr = 0;
  if (network_status(TNFS_MISSING, &bw, &conn, &nerr) == FN_ERR_OK)
    printf("  status err=%u (170 = file not found)\n", nerr);
#endif /* BUILD_APPLE2 */
  network_close(TNFS_MISSING);

  END_OF_TEST();
}

void test_regr_bad_open_then_write(void)
{
  uint8_t err;

  SECTION("write after a failed open");

#ifdef FN_BROKEN_network_open_invalid
  SKIP(network_open_invalid);
#else
  /* a6b481cbd (iwm) / 8a2345741 (adamnet): a bad open must be answered
   * at bus level, and a write on the dead channel must error cleanly
   * instead of crashing the firmware */
  err = network_open("N1:ZZZ://nowhere/", OPEN_MODE_RW, OPEN_TRANS_NONE);
#ifndef BUILD_APPLE2
  TEST("open with bogus scheme fails", err != FN_ERR_OK);
#endif /* ! BUILD_APPLE2 */

  err = network_write("N1:ZZZ://nowhere/", (const uint8_t *) "hi", 2);
  TEST("write on dead channel errors cleanly", err != FN_ERR_OK);

  err = network_close("N1:ZZZ://nowhere/");
  TEST("close after failed open completes", err == FN_ERR_OK);
#endif /* FN_BROKEN_network_open_invalid */

  END_OF_TEST();
}

void test_regr_dir_listing_width(void)
{
  SECTION("long directory entry width");

#if defined(BUILD_ADAM) || defined(BUILD_COCO)
  {
    uint8_t err;
    int16_t r;
    uint16_t i;
#ifdef BUILD_ADAM
    uint16_t want = 30;
#else
    uint16_t want = 31;
#endif

    /* 31196e482: LONG format entries are 30 columns on ADAM and 31 on
     * CoCo instead of a fixed 37 */
    err = network_open(TNFS_ROOT, 0x06, 0x80);
    TEST("open TNFS root as LONG listing", err == FN_ERR_OK);

    memset(g.net, 0, sizeof(g.net));
    r = network_read(TNFS_ROOT, g.net, sizeof(g.net) - 1);
    TEST("read LONG listing", r > 0);

    /* Measure the first line */
    for (i = 0; i < (uint16_t) r; i++) {
      if (g.net[i] == '\r' || g.net[i] == '\n' || g.net[i] == 0x9B)
        break;
    }
    printf("  first entry is %u chars (want %u)\n", i, want);
    TEST("entry width matches platform", i == want);

    err = network_close(TNFS_ROOT);
    TEST("close listing", err == FN_ERR_OK);
  }
#else
  SKIP_WHY(dir_listing_width, "adam and coco only");
#endif /* BUILD_ADAM || BUILD_COCO */

  END_OF_TEST();
}

void test_regr_stale_devicespec(void)
{
  uint8_t err;
  int16_t r;
  uint16_t i;

  SECTION("shorter open after longer open");

  /* 232612375: the devicespec buffer must be cleared between opens, or
   * the shorter URL inherits the tail of the longer one */
  err = network_open(REGR_LONG_URL, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE);
  TEST("open longer URL", err == FN_ERR_OK);
  r = network_read(REGR_LONG_URL, g.net, 26);
  TEST("read longer URL body", r == 26);
  for (i = 0; i < 26; i++) {
    if (g.net[i] != RANGE_BYTE(i))
      break;
  }
  TEST("longer URL content correct", i == 26);
  network_close(REGR_LONG_URL);

  err = network_open(REGR_SHORT_URL, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE);
  TEST("open shorter URL", err == FN_ERR_OK);
  memset(g.net, 0, sizeof(g.net));
  r = network_read(REGR_SHORT_URL, g.net, 64);
  TEST("shorter URL resolves correctly", r > 0);
  network_close(REGR_SHORT_URL);

  END_OF_TEST();
}

void test_regr_adam_get_error(void)
{
  SECTION("adam extended error command");

#if defined(BUILD_ADAM) && defined(FN_NETWORK_UNIFIED_COMMANDS)
  {
    uint8_t e8;

    /* 819f1e938: $45 returns the latched error; raw call, no public API */
    e8 = 0;
    TEST("raw GET_ERROR completes",
         NETCALL_RV(FUJICMD_GET_ERROR, 1, &e8, 1));
    printf("  error=%u\n", e8);
  }
#else
  SKIP_WHY(adam_get_error, "adam only");
#endif /* BUILD_ADAM && FN_NETWORK_UNIFIED_COMMANDS */

  END_OF_TEST();
}

void test_regr_apple2_set_channel(void)
{
  SECTION("apple2 set-channel control");

#if defined(BUILD_APPLE2) && defined(FN_NETWORK_UNIFIED_COMMANDS)
  /* a6b481cbd: NET_SET_CHANNEL ($FA) previously never answered the bus,
   * timing out the host. Selecting unit 1 (already current) must ACK. */
  TEST("raw SET_CHANNEL completes", NETCALL_A1(FUJICMD_SET_CHANNEL, 1, 1));
#else
  SKIP_WHY(apple2_set_channel, "apple2 only");
#endif /* BUILD_APPLE2 && FN_NETWORK_UNIFIED_COMMANDS */

  END_OF_TEST();
}

void test_regr_lynx_alt_parse_query(void)
{
  SECTION("lynx alternate parse/query commands");

#if defined(BUILD_LYNX) && defined(FN_NETWORK_UNIFIED_COMMANDS)
  {
    uint8_t err;
    int16_t r;
    int16_t total;

    /* ComLynx also dispatches $80/$81 as aliases of PARSE/QUERY */
    err = network_open(NET_JSON_URL, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE);
    TEST("open JSON URL", err == FN_ERR_OK);

    err = network_set_channel_mode(NET_JSON_URL, NETWORK_CHANMODE_JSON);
    TEST("set JSON channel mode", err == FN_ERR_OK);

    TEST("raw PARSE_ALT completes", NETCALL(FUJICMD_PARSE_ALT, 1));
    TEST("raw QUERY_ALT completes",
         NETCALL_D(FUJICMD_QUERY_ALT, 1, "/slideshow/title",
                   sizeof("/slideshow/title") - 1));

    memset(g.net, 0, sizeof(g.net));
    total = 0;
    for (;;) {
      r = network_read_nb(NET_JSON_URL, &g.net[total], sizeof(g.net) - 1 - total);
      if (r < 1)
        break;
      total += r;
    }
    printf("  title=\"%s\"\n", (char *) g.net);
    TEST("alt query returned the title",
         t_contains((char *) g.net, "Sample Slide Show"));

    err = network_close(NET_JSON_URL);
    TEST("close JSON URL", err == FN_ERR_OK);
  }
#else
  SKIP_WHY(lynx_alt_parse_query, "lynx only");
#endif /* BUILD_LYNX && FN_NETWORK_UNIFIED_COMMANDS */

  END_OF_TEST();
}

/**
 * @file calmail.c
 * @brief Calendar (ICAL) and mail (IMAPS/GMAIL) protocol tests.
 *
 * Firmware regression commits exercised here (fujinet-firmware):
 *   83bff9a79  CALENDAR/GCAL/ICAL protocols -> test_cal_* (ICAL over the
 *              WebDAV fixture; GCAL needs Google auth: manual)
 *   c026d9a12  calendar compose/edit; CLOSE latches the commit error
 *              -> test_cal_write_open_fails covers error visibility;
 *                 a failing GCAL commit-on-close is a manual test
 *   7ad7113d3  mail protocol (GMAIL/IMAPS) -> test_mail_*, which need
 *              real credentials: compiled but SKIPped unless the build
 *              defines MAIL_TEST_CREDS, MAIL_IMAPS_URL ("N1:IMAPS://
 *              user:pass@host/INBOX") and MAIL_SMTP_URL.
 *
 * The calendar fixture is self-contained: a small .ics is PUT to the
 * public WebDAV server, then read back through the ICAL protocol.
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

#define OPEN_MODE_DIRECTORY (0x06)
/* aux2 for directory opens: 255 = raw structs, else text at width (low 7 bits) */
#define CAL_DIR_FORMAT_TEXT (128)
#define CAL_DIR_FORMAT_RAW  (255)

static const char cal_ics[] =
  "BEGIN:VCALENDAR\r\n"
  "VERSION:2.0\r\n"
  "PRODID:-//fujinet//lib-test//EN\r\n"
  "BEGIN:VEVENT\r\n"
  "UID:fnlib-test-1\r\n"
  "DTSTART:20260915T120000Z\r\n"
  "DTEND:20260915T130000Z\r\n"
  "SUMMARY:" CAL_EVENT_SUMMARY "\r\n"
  "END:VEVENT\r\n"
  "END:VCALENDAR\r\n";

/* Portable substring search over a byte buffer (result data is not
 * necessarily nul terminated) */
static uint8_t t_buf_contains(const uint8_t *hay, uint16_t hay_len, const char *needle)
{
  uint16_t i, j;

  for (i = 0; i < hay_len; i++) {
    for (j = 0; needle[j] && i + j < hay_len && hay[i + j] == (uint8_t) needle[j]; j++)
      ;
    if (!needle[j])
      return 1;
  }
  return 0;
}

/* IWM acknowledges every open at the bus level; the real result comes
 * back through status (same shape as fs.c's fs_open_result). */
static uint8_t cal_open_result(const char *devicespec, uint8_t err)
{
#ifdef BUILD_APPLE2
  uint16_t bw;
  uint8_t conn;
  uint8_t nerr;

  if (err != FN_ERR_OK)
    return err;
  if (network_status(devicespec, &bw, &conn, &nerr) != FN_ERR_OK)
    return FN_ERR_IO_ERROR;
  if (nerr == NETWORK_ERROR_END_OF_FILE)
    nerr = NETWORK_SUCCESS;
  if (nerr > NETWORK_SUCCESS && !bw)
    return nerr;
#endif /* BUILD_APPLE2 */
  return err;
}

void test_cal_put_ics(void)
{
  uint8_t err;

  SECTION("calendar fixture upload");

#ifdef FN_BROKEN_network_open
  SKIP(network_open);
#else
  err = network_open(CAL_ICS_PUT_URL, OPEN_MODE_HTTP_PUT, OPEN_TRANS_NONE);
  TEST("open .ics for PUT", err == FN_ERR_OK);

  err = network_http_put(CAL_ICS_PUT_URL, cal_ics);
  TEST("PUT .ics body", err == FN_ERR_OK);

  err = network_close(CAL_ICS_PUT_URL);
  TEST("close commits the PUT", err == FN_ERR_OK);
#endif /* FN_BROKEN_network_open */

  END_OF_TEST();
}

void test_cal_event_count(void)
{
  uint8_t err;
  int16_t r;

  SECTION("calendar event count (READ open)");

#ifdef FN_BROKEN_network_open
  SKIP(network_open);
#else
  err = cal_open_result(CAL_ICAL_URL,
                        network_open(CAL_ICAL_URL, OPEN_MODE_READ, OPEN_TRANS_NONE));
  TEST("open MONTH view for READ", err == FN_ERR_OK);

  memset(g.net, 0, sizeof(g.net));
  r = network_read(CAL_ICAL_URL, g.net, 8);
  TEST("read event count", r > 0);
  printf("  count=\"%s\"\n", (char *) g.net);
  TEST("one event in 2026-09", g.net[0] == '1');

  err = network_close(CAL_ICAL_URL);
  TEST("close view", err == FN_ERR_OK);
#endif /* FN_BROKEN_network_open */

  END_OF_TEST();
}

void test_cal_dir_text(void)
{
  uint8_t err;
  int16_t r;

  SECTION("calendar event index (text)");

#ifdef FN_BROKEN_network_open
  SKIP(network_open);
#else
  err = cal_open_result(CAL_ICAL_URL,
                        network_open(CAL_ICAL_URL, OPEN_MODE_DIRECTORY, CAL_DIR_FORMAT_TEXT));
  TEST("open MONTH view as text index", err == FN_ERR_OK);

  memset(g.net, 0, sizeof(g.net));
  r = network_read(CAL_ICAL_URL, g.net, sizeof(g.net) - 1);
  TEST("read text index", r > 0);
  printf("  index: %s\n", (char *) g.net);
  TEST("index names the event",
       t_buf_contains(g.net, (uint16_t) r, CAL_EVENT_SUMMARY));

  err = network_close(CAL_ICAL_URL);
  TEST("close view", err == FN_ERR_OK);
#endif /* FN_BROKEN_network_open */

  END_OF_TEST();
}

void test_cal_dir_raw(void)
{
  uint8_t err;
  int16_t r;

  SECTION("calendar event index (raw structs)");

#ifdef FN_BROKEN_network_open
  SKIP(network_open);
#else
  err = cal_open_result(CAL_ICAL_URL,
                        network_open(CAL_ICAL_URL, OPEN_MODE_DIRECTORY, CAL_DIR_FORMAT_RAW));
  TEST("open MONTH view as raw index", err == FN_ERR_OK);

  /* One CalEventItem is 277 bytes: u32 eventNum, u64 start, u64 end,
   * u8 flags, summary[96], location[64], category[32], uid[64] */
  memset(g.net, 0, sizeof(g.net));
  r = network_read(CAL_ICAL_URL, g.net, sizeof(g.net));
  printf("  raw index bytes=%d\n", (int) r);
  TEST("raw entry is at least one CalEventItem", r >= 277);
  TEST("raw entry carries the summary",
       t_buf_contains(g.net, (uint16_t) r, CAL_EVENT_SUMMARY));

  err = network_close(CAL_ICAL_URL);
  TEST("close view", err == FN_ERR_OK);
#endif /* FN_BROKEN_network_open */

  END_OF_TEST();
}

void test_cal_write_open_fails(void)
{
  uint8_t err;

  SECTION("calendar: ICAL is read-only");

#if defined(FN_BROKEN_network_open) || defined(FN_BROKEN_network_open_invalid)
  SKIP(network_open_invalid);
#else
  /* ICAL cannot compose (can_write() is false); a WRITE open must fail
   * and the error must be visible, not swallowed (c026d9a12/4e81a9815) */
  err = cal_open_result(CAL_ICAL_BASE,
                        network_open(CAL_ICAL_BASE, OPEN_MODE_WRITE, OPEN_TRANS_NONE));
  TEST("WRITE open of ICAL fails", err != FN_ERR_OK);
  network_close(CAL_ICAL_BASE);
#endif /* FN_BROKEN_network_open || FN_BROKEN_network_open_invalid */

  END_OF_TEST();
}

void test_mail_folder_dir(void)
{
  SECTION("mail folder index");

#ifndef MAIL_TEST_CREDS
  SKIP_WHY(mail_folder_dir, "needs MAIL_TEST_CREDS");
#else
  {
    uint8_t err;
    int16_t r;

    err = cal_open_result(MAIL_IMAPS_URL,
                          network_open(MAIL_IMAPS_URL, OPEN_MODE_DIRECTORY, CAL_DIR_FORMAT_TEXT));
    TEST("open INBOX index", err == FN_ERR_OK);

    memset(g.net, 0, sizeof(g.net));
    r = network_read(MAIL_IMAPS_URL, g.net, sizeof(g.net) - 1);
    TEST("read INBOX index", r > 0);
    printf("  %s\n", (char *) g.net);

    err = network_close(MAIL_IMAPS_URL);
    TEST("close INBOX index", err == FN_ERR_OK);
  }
#endif /* MAIL_TEST_CREDS */

  END_OF_TEST();
}

void test_mail_read_body(void)
{
  SECTION("mail message body");

#ifndef MAIL_TEST_CREDS
  SKIP_WHY(mail_read_body, "needs MAIL_TEST_CREDS");
#else
  {
    uint8_t err;
    int16_t r;

    err = cal_open_result(MAIL_IMAPS_URL "/1",
                          network_open(MAIL_IMAPS_URL "/1", OPEN_MODE_READ, OPEN_TRANS_NONE));
    TEST("open message 1", err == FN_ERR_OK);

    memset(g.net, 0, sizeof(g.net));
    r = network_read(MAIL_IMAPS_URL "/1", g.net, sizeof(g.net) - 1);
    TEST("read message body", r > 0);

    err = network_close(MAIL_IMAPS_URL "/1");
    TEST("close message", err == FN_ERR_OK);
  }
#endif /* MAIL_TEST_CREDS */

  END_OF_TEST();
}

void test_mail_compose(void)
{
  SECTION("mail compose");

#ifndef MAIL_TEST_CREDS
  SKIP_WHY(mail_compose, "needs MAIL_TEST_CREDS");
#else
  {
    uint8_t err;

    err = network_open(MAIL_SMTP_URL, OPEN_MODE_WRITE, OPEN_TRANS_NONE);
    TEST("open compose", err == FN_ERR_OK);

    err = network_write(MAIL_SMTP_URL, (const uint8_t *) "TO: " MAIL_TEST_TO "\r\n",
                        sizeof("TO: " MAIL_TEST_TO "\r\n") - 1);
    TEST("write TO header", err == FN_ERR_OK);
    err = network_write(MAIL_SMTP_URL, (const uint8_t *) "SUBJECT: fujinet-lib test\r\n\r\n",
                        sizeof("SUBJECT: fujinet-lib test\r\n\r\n") - 1);
    TEST("write SUBJECT and blank line", err == FN_ERR_OK);
    err = network_write(MAIL_SMTP_URL, (const uint8_t *) "Sent by the fujinet-lib test suite.\r\n",
                        sizeof("Sent by the fujinet-lib test suite.\r\n") - 1);
    TEST("write body", err == FN_ERR_OK);

    /* CLOSE commits the send */
    err = network_close(MAIL_SMTP_URL);
    TEST("close sends the message", err == FN_ERR_OK);
  }
#endif /* MAIL_TEST_CREDS */

  END_OF_TEST();
}

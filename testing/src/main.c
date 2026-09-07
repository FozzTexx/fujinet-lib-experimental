/**
 * @file fujinet_integration_tests.c
 * @brief Integration tests for FujiNet library: fujinet-fuji.h, fujinet-network.h, fujinet-clock.h
 *
 * These are integration tests that communicate over the actual FujiNet bus.
 * They CANNOT be run without a connected FujiNet device.
 *
 * Prerequisites:
 *   - A FujiNet device connected and accessible via its normal bus
 *   - At least one configured host slot (slot 0) pointing to a valid network share
 *   - WiFi configured and connected on the FujiNet device
 *   - Network access to fujinet.online from the FujiNet device
 *
 * Test output: PASS / FAIL lines to stdout, summary at end.
 * Exit code:   0 = all passed, non-zero = number of failures.
 *
 * Compiler constraints observed:
 *   - All variable declarations appear at the top of their enclosing block
 *   - No malloc/free; all buffers are static globals or small stack locals
 *   - Stack locals are kept under 64 bytes; anything larger is a static global
 */

#if !FUJI_TESTS && !NETWORK_TESTS && !NETWORK2_TESTS && !SGML_TESTS \
  && !CALMAIL_TESTS && !REGRESSION_TESTS && !CLOCK_TESTS && !DISK_TESTS \
  && !QRCODE_TESTS && !FS_TESTS && !APPKEY_TESTS && !DEVLIST_TESTS
#error "You need to choose some tests"
#endif

#include "broken.h"

#include "fuji.h"
#include "appkey.h"
#include "network.h"
#include "network2.h"
#include "sgml.h"
#include "calmail.h"
#include "regression.h"
#include "clock.h"
#include "fdsk.h"
#include "qrcode.h"
#include "fs.h"
#include "devlist.h"

#include "init_video.h"
#include "harness.h"

#ifdef _CMOC_VERSION_
#include <coco.h>
#else
#include <stdio.h>
#endif /* _CMOC_VERSION_ */

#ifdef BUILD_C64
#include "c64_find_load.h"
#endif /* BUILD_C64 */

/* =========================================================================
 * main
 * ========================================================================= */

int main(void)
{
  init_video();

  printf("=== FujiNet Integration Tests ===\n");
  printf("NOTE: Requires a connected, WiFi-enabled FujiNet device.\n");

  begin_testing();

#if FUJI_TESTS
  /* fuji.h */
  test_fuji_status();
  test_fuji_adapter_config();
  test_fuji_host_device_slots();
  test_fuji_device_filename();
  test_fuji_host_prefix();
  test_fuji_directory_ops();
  test_fuji_wifi();
  test_fuji_network_scan();
  test_fuji_guid();
  test_fuji_base64();
  test_fuji_hashing();
#endif // FUJI_TESTS

#if APPKEY_TESTS
  /* appkey.h */
  test_fuji_appkey();
#endif // APPKEY_TESTS

#if QR_TESTS
  test_fuji_qrcode();
#endif // QR_TESTS

#if DISK_TESTS
  /* fdsk.h */
  test_fuji_copy_file();
#endif // DISK_TESTS

#if QRCODE_TESTS
  /* qrcode.h */
  test_fuji_qrcode();
#endif // QRCODE_TESTS

#if NETWORK_TESTS
  /* network.h */
  test_network_init();
  test_network_write();
  test_network_status_after_close();
  test_network_open_no_n_prefix();
  test_network_set_eol();
#if ERROR_PATH_WORKS
  test_network_error_path();
#endif
  test_network_http_get();
  test_network_http_get_nonblocking();
  test_network_http_post();
  test_network_http_headers();
  test_network_http_channel_mode();
  test_network_json();
  test_network_http_put_plain();
  test_network_http_put_delete();
  test_multiple_network_devices();
  test_network_unit();
#endif // NETWORK_TESTS

#if NETWORK2_TESTS
  /* network2.h - the v5 command additions */
  test_net2_seek_tell();
  test_net2_seek_errors();
  test_net2_get_dstats();
  test_net2_set_translation();
  test_net2_set_eol_ext();
  test_net2_login_smoke();
  test_net2_timer_rate();
  test_net2_close_client();
  test_net2_udp_set_destination();
  test_net2_udp_get_remote();
  test_net2_channel_mode();
  test_net2_json_parameters();
  test_net2_read_count();
  test_net2_globals();
  test_net2_proceed();
#endif // NETWORK2_TESTS

#if SGML_TESTS
  /* sgml.h */
  test_sgml_parse_query();
  test_sgml_iterate();
  test_sgml_colon_selector();
  test_sgml_unsupported_rejected();
#endif // SGML_TESTS

#if CALMAIL_TESTS
  /* calmail.h */
  test_cal_put_ics();
  test_cal_event_count();
  test_cal_dir_text();
  test_cal_dir_raw();
  test_cal_write_open_fails();
  test_mail_folder_dir();
  test_mail_read_body();
  test_mail_compose();
#endif // CALMAIL_TESTS

#if REGRESSION_TESTS
  /* regression.h - test_regr_fs_no_open must stay first: it proves the
   * fs commands work with no protocol left over from an earlier open */
  test_regr_fs_no_open();
  test_regr_close_never_opened();
  test_regr_chdir_pwd_roundtrip();
  test_regr_open_error_status();
  test_regr_bad_open_then_write();
  test_regr_dir_listing_width();
  test_regr_stale_devicespec();
  test_regr_adam_get_error();
  test_regr_apple2_set_channel();
  test_regr_lynx_alt_parse_query();
#endif // REGRESSION_TESTS

#if FS_TESTS
  /* fs.h */
  test_fs_prefix();
  test_fs_make_test_dir();
  test_fs_create_files();
  test_fs_read_files();
  test_fs_rename_delete();
  test_fs_dir_lifecycle();
  test_fs_lock_unlock();
#endif // FS_TESTS

#if CLOCK_TESTS
  /* clock.h */
  test_clock_tz();
  test_clock_simple_binary();
  test_clock_prodos_binary();
  test_clock_apetime_binary();
  test_clock_tz_iso_string();
  test_clock_utc_iso_string();
  test_clock_apple3_sos();
  test_clock_simple_binary_with_hundredths();
  test_clock_get_time_tz();
#endif // CLOCK_TESTS

#if DEVLIST_TESTS
  /* devlist.h */
  test_fujinet_exists();
  test_clock_exists();
  test_network_exists();
#endif // DEVLIST_TESTS

  printf("\n=== All %d tests passed, %d skipped ===\n", tests_run, tests_skipped);
  print_versions();

  end_testing(0);
  return 0;
}

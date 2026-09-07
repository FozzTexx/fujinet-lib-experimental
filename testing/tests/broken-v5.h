#if FNLIB_VERSION_MAJOR >= 5

#ifdef BUILD_ADAM
#define FN_BROKEN_fuji_mount_host_slot_error
#define FN_BROKEN_network_open_invalid
#endif /* BUILD_ADAM */

#ifdef BUILD_C64
#define FN_BROKEN_network_open_invalid
#endif /* BUILD_C64 */

/* =====================================================================
 * FN_UNSUPPORTED_*: the firmware does not dispatch this network command
 * on this platform's bus. These are gaps to close in the network
 * unification work, not bugs; tests SKIP them via SKIP_UNSUPPORTED.
 * Delete a line when the bus gains the command.
 *
 * Source of truth: fujinet-firmware lib/device/<bus>/network.cpp command
 * dispatchers, cross-referenced against include/fujiCommandID.h.
 * ===================================================================== */

/* seek $25 / tell $26 / set-translation $54 / set-eol $4C /
 * set-timer-rate $5A exist only on SIO (atari) and RS232 (msdos, msx) */
#if defined(BUILD_APPLE2) || defined(BUILD_COCO) || defined(BUILD_ADAM) \
 || defined(BUILD_C64) || defined(BUILD_LYNX)
#define FN_UNSUPPORTED_network_seek
#define FN_UNSUPPORTED_network_tell
/* coco: the $54 dispatch arm was removed in fujinet-firmware e04b1759a */
#define FN_UNSUPPORTED_network_set_translation
#define FN_UNSUPPORTED_network_set_eol
#define FN_UNSUPPORTED_network_set_timer_rate
#endif

/* set-json-parameters $FB and the DSTATS inquiry $FF are SIO only */
#ifndef BUILD_ATARI
#define FN_UNSUPPORTED_network_json_set_parameters
#define FN_UNSUPPORTED_network_get_dstats
#endif /* ! BUILD_ATARI */

/* SGML channel mode (2) is not implemented on ComLynx or IEC */
#if defined(BUILD_LYNX) || defined(BUILD_C64)
#define FN_UNSUPPORTED_network_sgml
#endif

/* IEC speaks text commands on the command channel; it has no TCP
 * accept/close-client, no UDP, no HTTP channel mode command, and login
 * only via the text "login" command, not $FD/$FE */
#ifdef BUILD_C64
#define FN_UNSUPPORTED_network_accept
#define FN_UNSUPPORTED_network_close_client
#define FN_UNSUPPORTED_network_udp
#define FN_UNSUPPORTED_network_login
#define FN_UNSUPPORTED_network_http_chanmode
#endif /* BUILD_C64 */

/* UDP get-remote $72 is compiled out of ESP32 firmware on every bus.
 * Comment this out when testing against FujiNet-PC. */
#define FN_UNSUPPORTED_network_udp_get_remote

/* ComLynx READ always sends min(bytes waiting, 256), so a read asking
 * for less than is waiting cannot be satisfied */
#ifdef BUILD_LYNX
#define FN_UNSUPPORTED_network_read_partial
#endif /* BUILD_LYNX */

#endif // FNLIB_VERSION_MAJOR >= 5

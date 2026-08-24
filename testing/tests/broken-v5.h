#if FNLIB_VERSION_MAJOR >= 5

#ifdef BUILD_APPLE2
// Missing from firmware
#define FN_BROKEN_network_set_eol
#endif /* BUILD_APPLE2 */

#if defined(BUILD_ADAM) || defined(BUILD_ADAM_CPM)
// Missing from firmware
#define FN_BROKEN_fuji_status
#define FN_BROKEN_fuji_get_host_prefix
#define FN_BROKEN_fuji_get_wifi_enabled
#define FN_BROKEN_fuji_scan_for_networks
#define FN_BROKEN_network_set_eol
#endif /* BUILD_ADAM || BUILD_ADAM_CPM */

#ifdef BUILD_C64
// APETIMECMD_GET_SOS (0x53) collides with FUJICMD_STATUS, which iec handles
#define FN_BROKEN_clock_get_time_APPLE3_SOS_BINARY
#endif /* BUILD_C64 */

#ifdef BUILD_COCO
// Missing from firmware
#define FN_BROKEN_network_set_eol
#endif /* BUILD_COCO */

#endif // FNLIB_VERSION_MAJOR >= 5

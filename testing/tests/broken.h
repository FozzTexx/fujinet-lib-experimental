#ifndef BROKEN_H
#define BROKEN_H

/* =========================================================================
 * Known non-working functions
 *
 * Define a symbol here for each function that is known to be broken.
 * Tests that call a broken function will print SKIP instead of running.
 * Remove the #define once a function is fixed and re-tested.
 *
 * Example:
 *   #define FN_BROKEN_fuji_generate_guid
 *   #define FN_BROKEN_network_http_put
 * ========================================================================= */

#if FNLIB_VERSION_MAJOR < 5

#ifdef BUILD_ATARI
#define FN_BROKEN_fuji_open_directory2
#define FN_BROKEN_fuji_generate_guid
#endif /* BUILD_ATARI */

#ifdef BUILD_COCO
#define FN_BROKEN_fuji_status
#define FN_BROKEN_fuji_get_directory_position
#define FN_BROKEN_fuji_get_host_prefix
#define FN_BROKEN_fuji_base64_encode_output

#define FN_BROKEN_clock_get_time_PRODOS_BINARY
#define FN_BROKEN_clock_get_time_APETIME_BINARY
#define FN_BROKEN_clock_get_time_TZ_ISO_STRING
#define FN_BROKEN_clock_get_time_UTC_ISO_STRING
#define FN_BROKEN_clock_get_time_APPLE3_SOS_BINARY
#define FN_BROKEN_clock_get_tz
#define FN_BROKEN_clock_set_tz
#define FN_BROKEN_clock_get_time_tz
#endif /* BUILD_COCO */

#else /* FNLIB_VERSION_MAJOR >= 5 */
#endif /* FNLIB_VERSION_MAJOR */

#endif /* BROKEN_H */

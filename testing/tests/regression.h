#ifndef REGRESSION_H
#define REGRESSION_H

extern void test_regr_fs_no_open(void);
extern void test_regr_close_never_opened(void);
extern void test_regr_chdir_pwd_roundtrip(void);
extern void test_regr_open_error_status(void);
extern void test_regr_bad_open_then_write(void);
extern void test_regr_dir_listing_width(void);
extern void test_regr_stale_devicespec(void);
extern void test_regr_adam_get_error(void);
extern void test_regr_apple2_set_channel(void);
extern void test_regr_lynx_alt_parse_query(void);

#endif /* REGRESSION_H */

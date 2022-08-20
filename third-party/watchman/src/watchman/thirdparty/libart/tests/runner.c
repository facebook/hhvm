#include <check.h>
#include <stdio.h>
#include <syslog.h>
#include "test_art.c"

int main(void)
{
    setlogmask(LOG_UPTO(LOG_DEBUG));

    Suite *s1 = suite_create("art");
    TCase *tc1 = tcase_create("art");
    SRunner *sr = srunner_create(s1);
    int nf;

    // Add the art tests
    suite_add_tcase(s1, tc1);
    tcase_add_test(tc1, test_art_init_and_destroy);
    tcase_add_test(tc1, test_art_insert);
    tcase_add_test(tc1, test_art_insert_verylong);
    tcase_add_test(tc1, test_art_insert_search);
    tcase_add_test(tc1, test_art_insert_delete);
    tcase_add_test(tc1, test_art_insert_iter);
    tcase_add_test(tc1, test_art_iter_prefix);
    tcase_add_test(tc1, test_art_long_prefix);
    tcase_add_test(tc1, test_art_insert_search_uuid);

    srunner_run_all(sr, CK_ENV);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return nf == 0 ? 0 : 1;
}


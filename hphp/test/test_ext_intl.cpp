/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <test/test_ext_intl.h>
#include <runtime/ext/ext_intl.h>

IMPLEMENT_SEP_EXTENSION_TEST(Intl);
///////////////////////////////////////////////////////////////////////////////

bool TestExtIntl::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_intl_get_error_code);
  RUN_TEST(test_intl_get_error_message);
  RUN_TEST(test_intl_error_name);
  RUN_TEST(test_intl_is_failure);
  RUN_TEST(test_collator_asort);
  RUN_TEST(test_collator_compare);
  RUN_TEST(test_collator_create);
  RUN_TEST(test_collator_get_attribute);
  RUN_TEST(test_collator_get_error_code);
  RUN_TEST(test_collator_get_error_message);
  RUN_TEST(test_collator_get_locale);
  RUN_TEST(test_collator_get_strength);
  RUN_TEST(test_collator_set_attribute);
  RUN_TEST(test_collator_set_strength);
  RUN_TEST(test_collator_sort_with_sort_keys);
  RUN_TEST(test_collator_sort);
  RUN_TEST(test_idn_to_ascii);
  RUN_TEST(test_idn_to_unicode);
  RUN_TEST(test_idn_to_utf8);
  RUN_TEST(test_Collator);
  RUN_TEST(test_Locale);
  RUN_TEST(test_Normalizer);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtIntl::test_intl_get_error_code() {
  return Count(true);
}

bool TestExtIntl::test_intl_get_error_message() {
  return Count(true);
}

bool TestExtIntl::test_intl_error_name() {
  return Count(true);
}

bool TestExtIntl::test_intl_is_failure() {
  return Count(true);
}

bool TestExtIntl::test_collator_asort() {
  return Count(true);
}

bool TestExtIntl::test_collator_compare() {
  return Count(true);
}

bool TestExtIntl::test_collator_create() {
  return Count(true);
}

bool TestExtIntl::test_collator_get_attribute() {
  return Count(true);
}

bool TestExtIntl::test_collator_get_error_code() {
  return Count(true);
}

bool TestExtIntl::test_collator_get_error_message() {
  return Count(true);
}

bool TestExtIntl::test_collator_get_locale() {
  return Count(true);
}

bool TestExtIntl::test_collator_get_strength() {
  return Count(true);
}

bool TestExtIntl::test_collator_set_attribute() {
  return Count(true);
}

bool TestExtIntl::test_collator_set_strength() {
  return Count(true);
}

bool TestExtIntl::test_collator_sort_with_sort_keys() {
  return Count(true);
}

bool TestExtIntl::test_collator_sort() {
  return Count(true);
}

bool TestExtIntl::test_idn_to_ascii() {
  VS(f_idn_to_ascii("www.m\xc3\xa5nsjonasson.se"),
      "www.xn--mnsjonasson-x8a.se");
  VS(f_idn_to_ascii("www.facebook.com"),
      "www.facebook.com");
  VS(f_idn_to_ascii("www.xn--m\xc3\xa5nsjonasson.se"),
      false);
  VS(f_idn_to_ascii("www.12345678901234567890123456789"
                    "012345678901234m\xc3\xa5nsjonasson.se"),
      "www.xn--123456789012345678901234567890123456789"
      "01234mnsjonasson-5we.se");
  VS(f_idn_to_ascii("www.12345678901234567890123456789"
                    "0123456789012345m\xc3\xa5nsjonasson.se"),
      false);
  VS(f_idn_to_ascii("\u1937ai\u18ed-\u18f0.tk", 1),
    "xn--ai--youq53b.tk")
  VS(f_idn_to_ascii("\u1937ai\u18ed-\u18f0.tk", 0),
    false)
  return Count(true);
}

bool TestExtIntl::test_idn_to_unicode() {
  VS(f_idn_to_unicode("www.xn--mnsjonasson-x8a.se"),
      "www.m\xc3\xa5nsjonasson.se");
  VS(f_idn_to_unicode("www.facebook.com"),
      "www.facebook.com");
  VS(f_idn_to_unicode("www.xn--12345678901234567890123456789"
                      "012345678901234mnsjonasson-5we.se"),
      "www.12345678901234567890123456789"
      "012345678901234m\xc3\xa5nsjonasson.se");
  return Count(true);
}

bool TestExtIntl::test_idn_to_utf8() {
  VS(f_idn_to_utf8("www.xn--mnsjonasson-x8a.se"),
      "www.m\xc3\xa5nsjonasson.se");
  VS(f_idn_to_utf8("www.facebook.com"),
      "www.facebook.com");
  VS(f_idn_to_utf8("www.xn--12345678901234567890123456789"
                   "012345678901234mnsjonasson-5we.se"),
      "www.12345678901234567890123456789"
      "012345678901234m\xc3\xa5nsjonasson.se");
  return Count(true);
}

bool TestExtIntl::test_Collator() {
  return Count(true);
}

bool TestExtIntl::test_Locale() {
  return Count(true);
}

bool TestExtIntl::test_Normalizer() {
  VERIFY(c_Normalizer::ti_isnormalized("\xC3\x85"));
  VERIFY(!c_Normalizer::ti_isnormalized("A\xCC\x8A"));
  VS(c_Normalizer::ti_normalize("A\xCC\x8A", q_Normalizer$$FORM_C),
     "\xC3\x85");
  return Count(true);
}


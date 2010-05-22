/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <test/test_ext_iconv.h>
#include <runtime/ext/ext_iconv.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtIconv::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_iconv_mime_encode);
  RUN_TEST(test_iconv_mime_decode);
  RUN_TEST(test_iconv_mime_decode_headers);
  RUN_TEST(test_iconv_get_encoding);
  RUN_TEST(test_iconv_set_encoding);
  RUN_TEST(test_iconv);
  RUN_TEST(test_iconv_strlen);
  RUN_TEST(test_iconv_strpos);
  RUN_TEST(test_iconv_strrpos);
  RUN_TEST(test_iconv_substr);
  RUN_TEST(test_ob_iconv_handler);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtIconv::test_iconv_mime_encode() {
  Array preferences = CREATE_MAP4("input-charset", "ISO-8859-1",
                                  "output-charset", "UTF-8",
                                  "line-length", 76,
                                  "line-break-chars", "\n");
  preferences.set("scheme", "Q");
  VS(f_iconv_mime_encode("Subject", "Pr\xDC""fung Pr\xDC""fung", preferences),
     "Subject: =?UTF-8?Q?Pr=C3=9Cfung=20Pr=C3=9Cfung?=");

  preferences.set("scheme", "B");
  VS(f_iconv_mime_encode("Subject", "Pr\xDC""fung Pr\xDC""fung", preferences),
     "Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=");

  return Count(true);
}

bool TestExtIconv::test_iconv_mime_decode() {
  VS(f_iconv_mime_decode("Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=",
                         0, "ISO-8859-1"),
     "Subject: Pr\xDC""fung Pr\xDC""fung");

  return Count(true);
}

bool TestExtIconv::test_iconv_mime_decode_headers() {
  VS(f_iconv_mime_decode_headers
     ("Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=\n"
      "Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=\n",
      0, "ISO-8859-1"),
     CREATE_MAP1("Subject", CREATE_VECTOR2("Pr\xDC""fung Pr\xDC""fung",
                                           "Pr\xDC""fung Pr\xDC""fung")));

  return Count(true);
}

bool TestExtIconv::test_iconv_get_encoding() {
  VS(f_iconv_get_encoding(),
     CREATE_MAP3("input_encoding", "ISO-8859-1",
                 "output_encoding", "ISO-8859-1",
                 "internal_encoding", "ISO-8859-1"));

  return Count(true);
}

bool TestExtIconv::test_iconv_set_encoding() {
  VS(f_iconv_set_encoding("output_encoding", "UTF-8"), true);

  VS(f_iconv_get_encoding(),
     CREATE_MAP3("input_encoding", "ISO-8859-1",
                 "output_encoding", "UTF-8",
                 "internal_encoding", "ISO-8859-1"));

  return Count(true);
}

bool TestExtIconv::test_iconv() {
  VS(f_iconv("UTF-8", "BIG5", "\xE2\x82\xAC"), "\xa3\xe1");
  VS(f_iconv("ISO-8859-1", "UTF-8", "Pr\xDC""fung"), "Pr\xC3\x9C""fung");
  return Count(true);
}

bool TestExtIconv::test_iconv_strlen() {
  VS(f_iconv_strlen("Pr\xDC""fung", "ISO-8859-1"), 7);
  VS(f_iconv_strlen("Pr\xC3\x9C""fung", "UTF-8"), 7);
  return Count(true);
}

bool TestExtIconv::test_iconv_strpos() {
  VS(f_iconv_strpos("Pr\xC3\x9C\xC3\x9D""fung", "\xC3\x9D", 0, "UTF-8"), 3);
  return Count(true);
}

bool TestExtIconv::test_iconv_strrpos() {
  VS(f_iconv_strrpos("Pr\xC3\x9C""abc\xC3\x9C""fung", "\xC3\x9C", "UTF-8"), 6);
  return Count(true);
}

bool TestExtIconv::test_iconv_substr() {
  VS(f_iconv_substr("Pr\xC3\x9C\xC3\x9D""fung", 2, 2, "UTF-8"),
     "\xC3\x9C\xC3\x9D");
  return Count(true);
}

bool TestExtIconv::test_ob_iconv_handler() {
  // TODO: test this in TestServer
  return Count(true);
}

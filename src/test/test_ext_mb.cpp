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

#include <test/test_ext_mb.h>
#include <runtime/ext/ext_mb.h>
#include <runtime/ext/ext_string.h>
#include <runtime/ext/ext_array.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtMb::RunTests(const std::string &which) {
  bool ret = true;
  f_mb_internal_encoding("UTF-8");

  RUN_TEST(test_mb_list_encodings);
  RUN_TEST(test_mb_list_encodings_alias_names);
  RUN_TEST(test_mb_list_mime_names);
  RUN_TEST(test_mb_check_encoding);
  RUN_TEST(test_mb_convert_case);
  RUN_TEST(test_mb_convert_encoding);
  RUN_TEST(test_mb_convert_kana);
  RUN_TEST(test_mb_convert_variables);
  RUN_TEST(test_mb_decode_mimeheader);
  RUN_TEST(test_mb_decode_numericentity);
  RUN_TEST(test_mb_detect_encoding);
  RUN_TEST(test_mb_detect_order);
  RUN_TEST(test_mb_encode_mimeheader);
  RUN_TEST(test_mb_encode_numericentity);
  RUN_TEST(test_mb_ereg_match);
  RUN_TEST(test_mb_ereg_replace);
  RUN_TEST(test_mb_ereg_search_getpos);
  RUN_TEST(test_mb_ereg_search_getregs);
  RUN_TEST(test_mb_ereg_search_init);
  RUN_TEST(test_mb_ereg_search_pos);
  RUN_TEST(test_mb_ereg_search_regs);
  RUN_TEST(test_mb_ereg_search_setpos);
  RUN_TEST(test_mb_ereg_search);
  RUN_TEST(test_mb_ereg);
  RUN_TEST(test_mb_eregi_replace);
  RUN_TEST(test_mb_eregi);
  RUN_TEST(test_mb_get_info);
  RUN_TEST(test_mb_http_input);
  RUN_TEST(test_mb_http_output);
  RUN_TEST(test_mb_internal_encoding);
  RUN_TEST(test_mb_language);
  RUN_TEST(test_mb_output_handler);
  RUN_TEST(test_mb_parse_str);
  RUN_TEST(test_mb_preferred_mime_name);
  RUN_TEST(test_mb_regex_encoding);
  RUN_TEST(test_mb_regex_set_options);
  RUN_TEST(test_mb_send_mail);
  RUN_TEST(test_mb_split);
  RUN_TEST(test_mb_strcut);
  RUN_TEST(test_mb_strimwidth);
  RUN_TEST(test_mb_stripos);
  RUN_TEST(test_mb_stristr);
  RUN_TEST(test_mb_strlen);
  RUN_TEST(test_mb_strpos);
  RUN_TEST(test_mb_strrchr);
  RUN_TEST(test_mb_strrichr);
  RUN_TEST(test_mb_strripos);
  RUN_TEST(test_mb_strrpos);
  RUN_TEST(test_mb_strstr);
  RUN_TEST(test_mb_strtolower);
  RUN_TEST(test_mb_strtoupper);
  RUN_TEST(test_mb_strwidth);
  RUN_TEST(test_mb_substitute_character);
  RUN_TEST(test_mb_substr_count);
  RUN_TEST(test_mb_substr);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtMb::test_mb_list_encodings() {
  VERIFY(!same(f_array_search("UTF-8", f_mb_list_encodings()), false));
  return Count(true);
}

bool TestExtMb::test_mb_list_encodings_alias_names() {
  VS(f_mb_list_encodings_alias_names()["Quoted-Printable"],
     CREATE_VECTOR1("qprint"));
  return Count(true);
}

bool TestExtMb::test_mb_list_mime_names() {
  VS(f_mb_list_mime_names()["UUENCODE"], "x-uuencode");
  return Count(true);
}

bool TestExtMb::test_mb_check_encoding() {
  VERIFY(f_mb_check_encoding("Pr\xC3\x9C\xC3\x9D""fung", "UTF-8"));
  return Count(true);
}

bool TestExtMb::test_mb_convert_case() {
  String str = "mary had a Little lamb and she loved it so";
  str = f_mb_convert_case(str, k_MB_CASE_UPPER, "UTF-8");
  VS(str, "MARY HAD A LITTLE LAMB AND SHE LOVED IT SO");
  str = f_mb_convert_case(str, k_MB_CASE_TITLE, "UTF-8");
  VS(str, "Mary Had A Little Lamb And She Loved It So");
  return Count(true);
}

bool TestExtMb::test_mb_convert_encoding() {
  String str = "Pr\xC3\x9C""fung";

  VS(f_mb_convert_encoding(str, "ISO-8859-1", "UTF-8"),      "Pr\xDC""fung");
  VS(f_mb_convert_encoding(str, "ISO-8859-1", "UTF-8, JIS"), "Pr\xDC""fung");
  VS(f_mb_convert_encoding(str, "ISO-8859-1", "auto"),       "Pr\xDC""fung");

  return Count(true);
}

bool TestExtMb::test_mb_convert_kana() {
  VS(f_mb_convert_kana("foo"), "foo");
  return Count(true);
}

bool TestExtMb::test_mb_convert_variables() {
  Variant str  = "Pr\xC3\x9C""fung";
  Variant str1 = "Pr\xC3\x9C""fung";
  Variant str2 = "Pr\xC3\x9C""fung";
  Variant inputenc = f_mb_convert_variables(5, "ISO-8859-1", "UTF-8", ref(str),
                                            CREATE_VECTOR2(ref(str1),
                                                           ref(str2)));
  VS(str,  "Pr\xDC""fung");
  VS(str1, "Pr\xDC""fung");
  VS(str2, "Pr\xDC""fung");
  return Count(true);
}

bool TestExtMb::test_mb_decode_mimeheader() {
  f_mb_internal_encoding("ISO-8859-1");
  VS(f_mb_decode_mimeheader("Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=\n"),
     "Subject: Pr\xDC""fung Pr\xDC""fung");
  f_mb_internal_encoding("UTF-8");
  return Count(true);
}

bool TestExtMb::test_mb_decode_numericentity() {
  Array convmap = CREATE_VECTOR4(0x0, 0x2FFFF, 0, 0xFFFF);
  VS(f_mb_decode_numericentity("&#8217;&#7936;&#226;", convmap, "UTF-8"),
     "\xe2\x80\x99\xe1\xbc\x80\xc3\xa2");
  return Count(true);
}

bool TestExtMb::test_mb_detect_encoding() {
  String str = "Pr\xC3\x9C\xC3\x9D""fung";

  /* Detect character encoding with current detect_order */
  VS(f_mb_detect_encoding(str), "UTF-8");

  /* "auto" is expanded to "ASCII,JIS,UTF-8,EUC-JP,SJIS" */
  VS(f_mb_detect_encoding(str, "auto"), "UTF-8");

  /* Specify encoding_list character encoding by comma separated list */
  VS(f_mb_detect_encoding(str, "JIS, eucjp-win, sjis-win"), "SJIS-win");

  /* Use array to specify encoding_list  */
  Array ary = CREATE_VECTOR3("ASCII", "JIS", "EUC-JP");
  VS(f_mb_detect_encoding(str, ary), "EUC-JP");

  return Count(true);
}

bool TestExtMb::test_mb_detect_order() {
  String str = "Pr\xC3\x9C\xC3\x9D""fung";

  /* Set detection order by enumerated list */
  {
    f_mb_detect_order("eucjp-win,sjis-win,UTF-8");
    VS(f_mb_detect_encoding(str), "SJIS-win");
    f_mb_detect_order("eucjp-win,UTF-8,sjis-win");
    VS(f_mb_detect_encoding(str), "UTF-8");
  }

  /* Set detection order by array */
  {
    f_mb_detect_order(CREATE_VECTOR3("eucjp-win", "sjis-win", "UTF-8"));
    VS(f_mb_detect_encoding(str), "SJIS-win");
    f_mb_detect_order(CREATE_VECTOR3("eucjp-win", "UTF-8", "sjis-win"));
    VS(f_mb_detect_encoding(str), "UTF-8");
  }

  /* Display current detection order */
  VS(f_implode(", ", f_mb_detect_order()), "eucJP-win, UTF-8, SJIS-win");

  return Count(true);
}

bool TestExtMb::test_mb_encode_mimeheader() {
  f_mb_internal_encoding("ISO-8859-1");
  VS(f_mb_encode_mimeheader("Subject: Pr\xDC""fung Pr\xDC""fung",
                            "UTF-8", "B"),
     "Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=");
  f_mb_internal_encoding("UTF-8");
  return Count(true);
}

bool TestExtMb::test_mb_encode_numericentity() {
  Array convmap = CREATE_VECTOR4(0x0, 0x2FFFF, 0, 0xFFFF);
  VS(f_mb_encode_numericentity("\xe2\x80\x99\xe1\xbc\x80\xc3\xa2",
                               convmap, "UTF-8"),
     "&#8217;&#7936;&#226;");
  return Count(true);
}

bool TestExtMb::test_mb_ereg_match() {
  VERIFY(!f_mb_ereg_match("a", "some apples"));
  VERIFY(f_mb_ereg_match("a", "a kiwi"));
  VERIFY(f_mb_ereg_match(".*a", "some apples"));
  return Count(true);
}

bool TestExtMb::test_mb_ereg_replace() {
  {
    String str = "This is a test";
    VS(f_str_replace(" is", " was", str), "This was a test");
    VS(f_mb_ereg_replace("( )is", "\\1was", str), "This was a test");
    VS(f_mb_ereg_replace("(( )is)", "\\2was", str), "This was a test");
  }
  {
    int num = 4;
    String str = "This string has four words.";
    str = f_mb_ereg_replace("four", num, str);
    VS(str, "This string has 4 words.");
  }
  {
    String test = "http://test.com/test";
    test = f_mb_ereg_replace("[[:alpha:]]+://[^<>[:space:]]+[[:alnum:]/]",
                             "<a href=\"\\0\">\\0</a>", test);
    VS(test, "<a href=\"http://test.com/test\">http://test.com/test</a>");
  }
  return Count(true);
}

bool TestExtMb::test_mb_ereg_search_getpos() {
  String str = "Pr\xC3\x9C\xC3\x9D""fung abc p\xC3\x9C";
  String reg = "\\w+";
  f_mb_regex_encoding("UTF-8");
  f_mb_ereg_search_init(str, reg);
  Variant r = f_mb_ereg_search();
  r = f_mb_ereg_search_getregs(); // get first result
  VS(r, CREATE_VECTOR1("Pr\xC3\x9C\xC3\x9D""fung"));
  VS(f_mb_ereg_search_getpos(), 10);
  return Count(true);
}

bool TestExtMb::test_mb_ereg_search_getregs() {
  String str = "Pr\xC3\x9C\xC3\x9D""fung abc p\xC3\x9C";
  String reg = "\\w+";
  f_mb_regex_encoding("UTF-8");
  f_mb_ereg_search_init(str, reg);
  Variant r = f_mb_ereg_search();
  r = f_mb_ereg_search_getregs(); // get first result
  VS(r, CREATE_VECTOR1("Pr\xC3\x9C\xC3\x9D""fung"));
  return Count(true);
}

bool TestExtMb::test_mb_ereg_search_init() {
  VERIFY(f_mb_ereg_search_init("abcdefabcdabc", "abc"));
  return Count(true);
}

bool TestExtMb::test_mb_ereg_search_pos() {
  String str = "Pr\xC3\x9C\xC3\x9D""fung abc p\xC3\x9C";
  String reg = "\\w+";
  f_mb_regex_encoding("UTF-8");
  f_mb_ereg_search_init(str, reg);
  Variant r = f_mb_ereg_search();
  r = f_mb_ereg_search_getregs(); // get first result
  VS(r, CREATE_VECTOR1("Pr\xC3\x9C\xC3\x9D""fung"));
  VS(f_mb_ereg_search_pos(), CREATE_VECTOR2(11, 3));
  return Count(true);
}

bool TestExtMb::test_mb_ereg_search_regs() {
  String str = "Pr\xC3\x9C\xC3\x9D""fung abc p\xC3\x9C";
  String reg = "\\w+";
  f_mb_regex_encoding("UTF-8");
  f_mb_ereg_search_init(str, reg);
  Variant r = f_mb_ereg_search();
  r = f_mb_ereg_search_getregs(); // get first result
  VS(r, CREATE_VECTOR1("Pr\xC3\x9C\xC3\x9D""fung"));
  r = f_mb_ereg_search_regs();    // get next result
  VS(r, CREATE_VECTOR1("abc"));
  return Count(true);
}

bool TestExtMb::test_mb_ereg_search_setpos() {
  String str = "Pr\xC3\x9C\xC3\x9D""fung abc p\xC3\x9C";
  String reg = "\\w+";
  f_mb_regex_encoding("UTF-8");
  f_mb_ereg_search_init(str, reg);
  Variant r = f_mb_ereg_search();
  r = f_mb_ereg_search_getregs(); // get first result
  VS(r, CREATE_VECTOR1("Pr\xC3\x9C\xC3\x9D""fung"));
  VERIFY(f_mb_ereg_search_setpos(15));
  r = f_mb_ereg_search_regs();    // get next result
  VS(r, CREATE_VECTOR1("p\xC3\x9C"));
  return Count(true);
}

bool TestExtMb::test_mb_ereg_search() {
  String str = "Pr\xC3\x9C\xC3\x9D""fung abc p\xC3\x9C";
  String reg = "\\w+";
  f_mb_regex_encoding("UTF-8");
  f_mb_ereg_search_init(str, reg);
  Variant r = f_mb_ereg_search();
  r = f_mb_ereg_search_getregs(); // get first result
  VS(r, CREATE_VECTOR1("Pr\xC3\x9C\xC3\x9D""fung"));
  return Count(true);
}

bool TestExtMb::test_mb_ereg() {
  Variant regs;
  String date = "1973-04-30";
  VERIFY(f_mb_ereg("([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})", date, ref(regs)));
  VS(regs[3], "30");
  VS(regs[2], "04");
  VS(regs[1], "1973");
  VS(regs[0], "1973-04-30");
  return Count(true);
}

bool TestExtMb::test_mb_eregi_replace() {
  String pattern = "(>[^<]*)(suffix)";
  String replacement = "\\1<span class=\"search\">\\2</span>";
  String body = ">whateversuffix";
  body = f_mb_eregi_replace(pattern, replacement, body);
  VS(body, ">whatever<span class=\"search\">suffix</span>");
  return Count(true);
}

bool TestExtMb::test_mb_eregi() {
  String str = "XYZ";
  VERIFY(f_mb_eregi("z", str));
  return Count(true);
}

bool TestExtMb::test_mb_get_info() {
  VERIFY(!f_mb_get_info()["detect_order"].toArray().empty());
  return Count(true);
}

bool TestExtMb::test_mb_http_input() {
  // TODO: test this in TestServer
  VS(f_mb_http_input(), false);
  return Count(true);
}

bool TestExtMb::test_mb_http_output() {
  // TODO: test this in TestServer
  VS(f_mb_http_output(), "pass");
  return Count(true);
}

bool TestExtMb::test_mb_internal_encoding() {
  /* Set internal character encoding to UTF-8 */
  f_mb_internal_encoding("UTF-8");

  /* Display current internal character encoding */
  VS(f_mb_internal_encoding(), "UTF-8");
  return Count(true);
}

bool TestExtMb::test_mb_language() {
  VS(f_mb_language(), "uni");
  return Count(true);
}

bool TestExtMb::test_mb_output_handler() {
  // TODO: test this in TestServer
  return Count(true);
}

bool TestExtMb::test_mb_parse_str() {
  Variant output;
  f_mb_parse_str("first=value&arr[]=foo+bar&arr[]=baz", ref(output));
  VS(output["first"], "value");
  VS(output["arr[]"], "baz"); // bug in mb_parse_str not following PHP's
  return Count(true);
}

bool TestExtMb::test_mb_preferred_mime_name() {
  VS(f_mb_preferred_mime_name("sjis-win"), "Shift_JIS");
  return Count(true);
}

bool TestExtMb::test_mb_regex_encoding() {
  VERIFY(f_mb_regex_encoding("UTF-8"));
  VS(f_mb_regex_encoding(), "UTF-8");
  return Count(true);
}

bool TestExtMb::test_mb_regex_set_options() {
  VS(f_mb_regex_set_options(), "pr");
  VERIFY(f_mb_regex_set_options("pz"));
  VS(f_mb_regex_set_options(), "pz");
  return Count(true);
}

bool TestExtMb::test_mb_send_mail() {
  //VERIFY(f_mb_send_mail("hzhao@facebook.com", __func__, "test");
  return Count(true);
}

bool TestExtMb::test_mb_split() {
  String date = "04/30/1973";
  Array ret = f_mb_split("[/.-]", date);
  VS(ret[0], "04");
  VS(ret[1], "30");
  VS(ret[2], "1973");
  return Count(true);
}

bool TestExtMb::test_mb_strcut() {
  VS(f_mb_strcut("abcdef", 1), "bcdef");
  VS(f_mb_strcut("abcdef", 1, 3), "bcd");
  VS(f_mb_strcut("abcdef", 0, 4), "abcd");
  VS(f_mb_strcut("abcdef", 0, 8), "abcdef");
  VS(f_mb_strcut("abcdef", -1, 1), "f");

  VS(f_mb_strcut("\xC3\x9C""bcdef", 2), "bcdef");
  VS(f_mb_strcut("\xC3\x9C""bcdef", 2, 3), "bcd");
  VS(f_mb_strcut("\xC3\x9C""bcdef", 0, 4), "\xC3\x9C""bc");
  VS(f_mb_strcut("\xC3\x9C""bcdef", 0, 8), "\xC3\x9C""bcdef");
  VS(f_mb_strcut("\xC3\x9C""bcdef", -1, 1), "f");

  return Count(true);
}

bool TestExtMb::test_mb_strimwidth() {
  VS(f_mb_strimwidth("long string", 0, 6, "..>"), "lon..>");
  VS(f_mb_strimwidth("\xC3\x9C""long string", 0, 6, "..>"), "\xC3\x9C""lo..>");
  return Count(true);
}

bool TestExtMb::test_mb_stripos() {
  VS(f_mb_stripos("abcdef abcdef", "A", 1), 7);
  VS(f_mb_stripos("abcdef\xC3\x9C""abcdef", "A", 1), 7);
  return Count(true);
}

bool TestExtMb::test_mb_stristr() {
  VS(f_mb_stristr("Hello World!", "earth"), false);
  return Count(true);
}

bool TestExtMb::test_mb_strlen() {
  VS(f_mb_strlen("test"), 4);
  VS(f_mb_strlen("Pr\xC3\x9C\xC3\x9D""fung"), 8);
  return Count(true);
}

bool TestExtMb::test_mb_strpos() {
  VS(f_mb_strpos("abcdef abcdef", "a", 1), 7);
  VS(f_mb_strpos("abcdef\xC3\x9C""abcdef", "a", 1), 7);
  VS(f_mb_strpos("abcdef\xC3\x9C""abcdef", "A", 1), false);
  return Count(true);
}

bool TestExtMb::test_mb_strrchr() {
  {
    String text = "Line 1\nLine 2\nLine 3";
    VS(f_mb_strrchr(text, "\n"), "\nLine 3");
  }
  {
    String text = "Line 1\nLine 2\xC3\x9C""Line 3";
    VS(f_strrchr(text, "\x9C"), "\x9C""Line 3");
    VS(f_mb_strrchr(text, "\x9C"), false);
  }
  return Count(true);
}

bool TestExtMb::test_mb_strrichr() {
  {
    String text = "Line 1\nLine 2\nLine 3";
    VS(f_mb_strrichr(text, "l"), "Line 3");
  }
  return Count(true);
}

bool TestExtMb::test_mb_strripos() {
  VS(f_mb_strripos("abcdef abcdef", "A"), 7);
  VS(f_mb_strripos("abcdef\xC3\x9C""abcdef", "A"), 7);
  return Count(true);
}

bool TestExtMb::test_mb_strrpos() {
  VS(f_mb_strrpos("abcdef abcdef", "a"), 7);
  VS(f_mb_strrpos("abcdef\xC3\x9C""abcdef", "a"), 7);
  return Count(true);
}

bool TestExtMb::test_mb_strstr() {
  String email  = "name@example.com";
  VS(f_mb_strstr(email, "@"), "@example.com");
  return Count(true);
}

bool TestExtMb::test_mb_strtolower() {
  String str = "Mary Had A Little Lamb and She LOVED It So";
  str = f_mb_strtolower(str);
  VS(str, "mary had a little lamb and she loved it so");
  VS(f_mb_strtolower("ABC"), "abc");
  return Count(true);
}

bool TestExtMb::test_mb_strtoupper() {
  String str = "Mary Had A Little Lamb and She LOVED It So";
  str = f_mb_strtoupper(str);
  VS(str, "MARY HAD A LITTLE LAMB AND SHE LOVED IT SO");
  VS(f_mb_strtoupper("abc"), "ABC");
  return Count(true);
}

bool TestExtMb::test_mb_strwidth() {
  VS(f_mb_strwidth("Pr\xC3\x9C""fung"), 7);
  return Count(true);
}

bool TestExtMb::test_mb_substitute_character() {
  /* Set with Unicode U+3013 (GETA MARK) */
  f_mb_substitute_character(0x3013);
  VS(f_mb_substitute_character(), 0x3013);

  /* Set hex format */
  f_mb_substitute_character("long");

  /* Display current setting */
  VS(f_mb_substitute_character(), "long");
  return Count(true);
}

bool TestExtMb::test_mb_substr_count() {
  VS(f_mb_substr_count("This is a test", "is"), 2);
  String text = "This is a test";
  VS(f_mb_substr_count(text, "is"), 2);

  // different from substr_count
  VS(f_mb_substr_count("gcdgcdgcd", "gcdgcd"), 2);
  return Count(true);
}

bool TestExtMb::test_mb_substr() {
  VS(f_mb_substr("abcdef", 1), "bcdef");
  VS(f_mb_substr("abcdef", 1, 3), "bcd");
  VS(f_mb_substr("abcdef", 0, 4), "abcd");
  VS(f_mb_substr("abcdef", 0, 8), "abcdef");
  VS(f_mb_substr("abcdef", -1, 1), "f");

  VS(f_mb_substr("\xC3\x9C""bcdef", 1), "bcdef");
  VS(f_mb_substr("\xC3\x9C""bcdef", 1, 3), "bcd");
  VS(f_mb_substr("\xC3\x9C""bcdef", 0, 4), "\xC3\x9C""bcd");
  VS(f_mb_substr("\xC3\x9C""bcdef", 0, 8), "\xC3\x9C""bcdef");
  VS(f_mb_substr("\xC3\x9C""bcdef", -1, 1), "f");
  return Count(true);
}

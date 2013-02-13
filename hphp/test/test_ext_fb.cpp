/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <test/test_ext_fb.h>
#include <runtime/ext/ext_fb.h>
#include <runtime/base/runtime_option.h>
#include <runtime/ext/ext_iconv.h>


///////////////////////////////////////////////////////////////////////////////

bool TestExtFb::RunTests(const std::string &which) {
  bool ret = true;

  DECLARE_TEST_FUNCTIONS("function test($s1) {"
                         " return $s1;"
                         "}");

  RUN_TEST(test_fb_compact_serialize);
  RUN_TEST(test_fb_compact_unserialize);
  RUN_TEST(test_fb_thrift_serialize);
  RUN_TEST(test_fb_thrift_unserialize);
  RUN_TEST(test_fb_rename_function);
  RUN_TEST(test_fb_utf8ize);
  RUN_TEST(test_fb_utf8_strlen);
  RUN_TEST(test_fb_utf8_strlen_deprecated);
  RUN_TEST(test_fb_utf8_substr);
  RUN_TEST(test_fb_call_user_func_safe);
  RUN_TEST(test_fb_call_user_func_safe_return);
  RUN_TEST(test_fb_call_user_func_array_safe);
  RUN_TEST(test_fb_load_local_databases);
  RUN_TEST(test_fb_parallel_query);
  RUN_TEST(test_fb_crossall_query);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

#define fb_cs_test(v) do {                                              \
    Variant ret;                                                        \
    Variant v_ = v;                                                     \
    Variant s_ = f_fb_compact_serialize(v_);                            \
    VERIFY(s_.isString());                                              \
    String ss_ = s_.toString();                                         \
    VERIFY(!ss_.empty());                                               \
    /* check high bit of first character always set */                  \
    VERIFY(ss_[0] & 0x80);                                              \
    VS(f_fb_compact_unserialize(s_, ref(ret)), v_);                     \
    VERIFY(same(ret, true));                                            \
    ret = null;                                                         \
    VS(f_fb_unserialize(s_, ref(ret)), v_);                             \
    VERIFY(same(ret, true));                                            \
  } while(0)

bool TestExtFb::test_fb_compact_serialize() {
  fb_cs_test(null);
  fb_cs_test(true);
  fb_cs_test(false);
  fb_cs_test(1234.5678);
  fb_cs_test("");
  fb_cs_test("a");
  fb_cs_test("\0");
  fb_cs_test("\0 a");
  fb_cs_test("0123012301230123");
  fb_cs_test("0123012301230123a");
  fb_cs_test("012301230123012");
  fb_cs_test(Array());
  fb_cs_test(CREATE_VECTOR1(12345));
  fb_cs_test(CREATE_VECTOR3(12345,"abc",0.1234));
  fb_cs_test(CREATE_MAP1(1, 12345));
  fb_cs_test(CREATE_MAP3(1, 12345, "a", 123124, "sdf", 0.1234));
  fb_cs_test(CREATE_VECTOR1(CREATE_VECTOR1("a")));
  fb_cs_test(CREATE_VECTOR2(1, CREATE_VECTOR1("a")));
  fb_cs_test(CREATE_VECTOR2(CREATE_VECTOR1("a"), 1));
  fb_cs_test(CREATE_VECTOR2(CREATE_VECTOR1("a"), CREATE_VECTOR1(1)));

  // Test skips
  fb_cs_test(CREATE_MAP3(0, "a", 1, "b", 3, "c"));
  fb_cs_test(CREATE_MAP3(1, "a", 2, "b", 3, "c"));
  fb_cs_test(CREATE_MAP3(0, "a", 2, "b", 3, "c"));
  fb_cs_test(CREATE_MAP1(3, "a"));
  // Test for overflow
  fb_cs_test(CREATE_MAP1((int64)((1ULL << 63) - 1), "a"));

  // Test each power of two, +/- 1 and the negatives of them
  // Test a single number and packed inside an array
  for (int i = 0; i < 64; ++i) {
    int64 n = (1ULL << i);
    fb_cs_test(n);    fb_cs_test(CREATE_VECTOR1(n));
    fb_cs_test(n-1);  fb_cs_test(CREATE_VECTOR1(n-1));
    fb_cs_test(n+1);  fb_cs_test(CREATE_VECTOR1(n+1));
    fb_cs_test(-n);   fb_cs_test(CREATE_VECTOR1(-n));
    fb_cs_test(-n-1); fb_cs_test(CREATE_VECTOR1(-n-1));
    fb_cs_test(-n+1); fb_cs_test(CREATE_VECTOR1(-n+1));
  }

  // Test vector code (PHP can't create those, but they might come form
  // C++ code in serialized strings)
  String s("\xfe\x01\x02\x03\xfc");  // VECTOR, 1, 2, 3, STOP
  Variant ret;
  VS(f_fb_compact_unserialize(s, ref(ret)), CREATE_VECTOR3(1, 2, 3));

  return Count(true);
}

#undef fb_cs_test

bool TestExtFb::test_fb_compact_unserialize() {
  // tested above
  return Count(true);
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtFb::test_fb_thrift_serialize() {
  Variant ret;
  VS(f_fb_thrift_unserialize(f_fb_thrift_serialize("test"), ref(ret)), "test");
  VERIFY(same(ret, true));

  ret = null;
  VS(f_fb_thrift_unserialize(f_fb_thrift_serialize(CREATE_VECTOR1("test")),
                             ref(ret)),
     CREATE_VECTOR1("test"));
  VERIFY(same(ret, true));
  return Count(true);
}

bool TestExtFb::test_fb_thrift_unserialize() {
  // tested above
  return Count(true);
}

bool TestExtFb::test_fb_rename_function() {
  // tested in TestCodeRun
  return Count(true);
}

// This string includes an invalid UTF-8 sequence.  The first byte
// suggests this is a three-byte UTF-8 sequence, but it's not valid.
//
// The legacy fb_utf8ize() implementation used to treat this three-byte
// sequence as two invalid code points followed by a valid ASCII character.
// It transforms the three bytes to the three code point sequence
// \xef\xbf\xbd\xef\xbf\xbd\x28.
//
// ICU treats this three-byte sequence as one invalid two-byte code point
// followed by a valid ASCII character.  An ICU-based fb_utf8ize()
// implementation will transform the three bytes to the two code point
// sequence \xef\xbf\xbd\x28.
static const char *INVALID_UTF_8_STRING = "\xe2\x82\x28";

bool TestExtFb::test_fb_utf8ize() {
  for (int i = 0; i < 2; i++) {
    RuntimeOption::Utf8izeReplace = (i == 0);
    {
      Variant s = "hon\xE7k";
      VERIFY(f_fb_utf8ize(ref(s)));
      if (RuntimeOption::Utf8izeReplace) {
        VS(s, "hon\uFFFDk");
      } else {
        VS(s, "honk");
      }
    }
    {
      Variant s = "test\xE0\xB0\xB1\xE0";
      VERIFY(f_fb_utf8ize(ref(s)));
      if (RuntimeOption::Utf8izeReplace) {
        VS(s, "test\xE0\xB0\xB1\uFFFD");
      } else {
        VS(s, "test\xE0\xB0\xB1");
      }
    }
    {
      Variant s = "test\xE0\xB0\xB1\xE0\xE0";
      VERIFY(f_fb_utf8ize(ref(s)));
      if (RuntimeOption::Utf8izeReplace) {
        VS(s, "test\xE0\xB0\xB1\uFFFD\uFFFD");
      } else {
        VS(s, "test\xE0\xB0\xB1");
      }
    }
    {
      Variant s = "\xfc";
      VERIFY(f_fb_utf8ize(ref(s)));
      if (RuntimeOption::Utf8izeReplace) {
        VS(s, "\uFFFD");
      } else {
        VS(s, "");
      }
    }
    {
      Variant s = "\xfc\xfc";
      VERIFY(f_fb_utf8ize(ref(s)));
      if (RuntimeOption::Utf8izeReplace) {
        VS(s, "\uFFFD\uFFFD");
      } else {
        VS(s, "");
      }
    }
    {
      // We intentionally consider null bytes invalid sequences.
      Variant s = String("abc\0def", 7, AttachLiteral);
      VERIFY(f_fb_utf8ize(ref(s)));
      if (RuntimeOption::Utf8izeReplace) {
        VS(s, "abc\uFFFD""def");
      } else {
        VS(s, "abcdef");
      }
    }
    {
      // ICU treats this as as two code points.
      // The old implementation treated this as three code points.
      Variant s = INVALID_UTF_8_STRING;
      VERIFY(f_fb_utf8ize(ref(s)));
      if (RuntimeOption::Utf8izeReplace) {
        VS(s, "\uFFFD""\x28");
      } else {
        VS(s, "\x28");
      }
    }
  }
  return Count(true);
}

// fb_utf8_strlen_deprecated() returns byte count on invalid input.
bool TestExtFb::test_fb_utf8_strlen_deprecated() {
  // Invalid UTF-8 sequence fails.
  VS(f_fb_utf8_strlen_deprecated(INVALID_UTF_8_STRING), 3);
  return Count(true);
}

bool TestExtFb::test_fb_utf8_strlen() {
  VS(f_fb_utf8_strlen(""), 0);
  VS(f_fb_utf8_strlen("a"), 1);
  VS(f_fb_utf8_strlen("ab"), 2);
  // Valid UTF-8 sequence returns code point count.
  VS(f_fb_utf8_strlen("\ub098\ub294"), 2);
  VS(f_fb_utf8_strlen(INVALID_UTF_8_STRING), 2);
  for (int i = 0; i < 2; i++) {
    // Test utf8ize() handling of invalid UTF-8 sequences and how
    // fb_utf8_strlen() counts them.
    // RuntimeOption::Utf8izeReplace set to non-zero value replaces invalid
    // bytes, including '\0' with a special UTF-8 code point: "\uFFFD".
    // RuntimeOption::Utf8izeReplace set to zero deletes the invalid
    // byte then continues parsing.
    RuntimeOption::Utf8izeReplace = (i == 0);
    {
      Variant s = String("abc\0def", 7, AttachLiteral);
      VS(s.toString().size(), 7);
      VS(f_fb_utf8_strlen(s), 7);

      f_fb_utf8ize(ref(s)); // Modifies s
      int ret = s.toString().size();
      if (RuntimeOption::Utf8izeReplace) {
        VS(ret, 9); // '\0' converted to "\uFFFD"
      } else {
        VS(ret, 6); // '\0' deleted from s
      }
      ret = f_fb_utf8_strlen(s);
      if (RuntimeOption::Utf8izeReplace) {
        VS(ret, 7); // '\0' and "\uFFFD" are both one code point, so no change
      } else {
        VS(ret, 6); // '\0' deleted, so one fewer code point
      }
    }
  }
  return Count(true);
}

bool TestExtFb::test_fb_utf8_substr() {
  // Falsey inputs
  VS(f_fb_utf8_substr("", 0, 0), false);
  VS(f_fb_utf8_substr("", 0, 1), false);
  VS(f_fb_utf8_substr("hon\xE7k", 0, INT_MAX), "hon\uFFFDk");
  VS(f_fb_utf8_substr("hon\xE7k", 0, 3), "hon"); // Never hits invalid byte
  VS(f_fb_utf8_substr("hon\xE7k", -4, INT_MAX), "on\uFFFDk");

  // Common cases
  VS(f_fb_utf8_substr("X", 0, 1), "X");
  VS(f_fb_utf8_substr("Hello", 0, INT_MAX), "Hello");
  VS(f_fb_utf8_substr("Hello", 1, 2), "el");
  VS(f_fb_utf8_substr("Pr\u00DC\u00DDx", 2, 2), "\u00DC\u00DD");

  // Negative start
  VS(f_fb_utf8_substr("abcdef", -1, INT_MAX), "f");
  VS(f_fb_utf8_substr("abcdef", -2, INT_MAX), "ef");
  VS(f_fb_utf8_substr("abcdef", -3, 1), "d");
  VS(f_fb_utf8_substr("", -1, 1), false);
  VS(f_fb_utf8_substr("X", -1, 1), "X");
  VS(f_fb_utf8_substr("XY", -1, 1), "Y");
  VS(f_fb_utf8_substr("Pr\u00DC\u00DDx", -3, 2), "\u00DC\u00DD");

  // Negative lengths
  VS(f_fb_utf8_substr("abcdef", 0, -1), "abcde");
  VS(f_fb_utf8_substr("abcdef", 2, -1), "cde");
  VS(f_fb_utf8_substr("abcdef", 4, -4), false); // nothing to return
  VS(f_fb_utf8_substr("abcdef", -3, -1), "de");

  // Invalid sequence
  VS(f_fb_utf8_substr(INVALID_UTF_8_STRING, 0), "\uFFFD\x28");

  return Count(true);
}

bool TestExtFb::test_fb_call_user_func_safe() {
  {
    Variant ret = f_fb_call_user_func_safe
      (1, "TEst", CREATE_VECTOR1("param"));
    VS(ret, CREATE_VECTOR2(true, "param"));
  }
  {
    Variant ret = f_fb_call_user_func_safe
      (1, "NonTEst", CREATE_VECTOR1("param"));
    VS(ret, CREATE_VECTOR2(false, null));
  }
  return Count(true);
}

bool TestExtFb::test_fb_call_user_func_safe_return() {
  {
    Variant ret = f_fb_call_user_func_safe_return
      (1, "TEst", "ok", CREATE_VECTOR1("param"));
    VS(ret, "param");
  }
  {
    Variant ret = f_fb_call_user_func_safe_return
      (1, "NonTEst", "ok", CREATE_VECTOR1("param"));
    VS(ret, "ok");
  }
  return Count(true);
}

bool TestExtFb::test_fb_call_user_func_array_safe() {
  {
    Variant ret = f_fb_call_user_func_array_safe
      ("TEst", CREATE_VECTOR1("param"));
    VS(ret, CREATE_VECTOR2(true, "param"));
  }
  {
    Variant ret = f_fb_call_user_func_array_safe
      ("NonTest", CREATE_VECTOR1("param"));
    VS(ret, CREATE_VECTOR2(false, null));
  }
  return Count(true);
}

bool TestExtFb::test_fb_load_local_databases() {
  // tested with PHP unit tests
  return Count(true);
}

bool TestExtFb::test_fb_parallel_query() {
  // tested with PHP unit tests
  return Count(true);
}

bool TestExtFb::test_fb_crossall_query() {
  // tested with PHP unit tests
  return Count(true);
}

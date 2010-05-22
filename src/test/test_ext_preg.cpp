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

#include <test/test_ext_preg.h>
#include <runtime/ext/ext_preg.h>
#include <runtime/ext/ext_array.h>
#include <runtime/ext/ext_string.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtPreg::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_preg_grep);
  RUN_TEST(test_preg_match);
  RUN_TEST(test_preg_match_all);
  RUN_TEST(test_preg_replace);
  RUN_TEST(test_preg_replace_callback);
  RUN_TEST(test_preg_split);
  RUN_TEST(test_preg_quote);
  RUN_TEST(test_preg_last_error);
  RUN_TEST(test_ereg_replace);
  RUN_TEST(test_eregi_replace);
  RUN_TEST(test_ereg);
  RUN_TEST(test_eregi);
  RUN_TEST(test_split);
  RUN_TEST(test_spliti);
  RUN_TEST(test_sql_regcase);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtPreg::test_preg_grep() {
  Array array = CREATE_VECTOR4("foo 123.1", "fg 24bar", "123.1", "24");
  Array fl_array = f_preg_grep("/^(\\d+)?\\.\\d+$/", array);
  VS(fl_array.size(), 1);
  VS(fl_array[2], "123.1");

  VS(f_preg_grep("/a/", CREATE_VECTOR2("c", "b")), Array::Create());
  return Count(true);
}

bool TestExtPreg::test_preg_match() {
  // The "i" after the pattern delimiter indicates a case-insensitive search
  VERIFY(f_preg_match("/php/i", "PHP is a scripting language."));

  // The \b in the pattern indicates a word boundary, so only the distinct
  // word "web" is matched, and not a word partial like "webbing" or "cobweb"
  VERIFY(f_preg_match("/\\bweb\\b/i", "is the web scripting"));

  // get host name from URL
  Variant matches;
  f_preg_match("@^(?:http://)?([^/]+)@i",
               "http://www.php.net/index.html", ref(matches));
  String host = matches[1];
  VS(host, "www.php.net");

  // get last two segments of host name
  f_preg_match("/[^.]+\\.[^.]+$/", host, ref(matches));
  VS(matches[0], "php.net");

  String str = "foobar: 2008";
  f_preg_match("/(?<name>\\w+): (?<digit>\\d+)/", str, ref(matches));
  VS(f_print_r(matches, true),
     "Array\n"
     "(\n"
     "    [0] => foobar: 2008\n"
     "    [name] => foobar\n"
     "    [1] => foobar\n"
     "    [digit] => 2008\n"
     "    [2] => 2008\n"
     ")\n");

  return Count(true);
}

bool TestExtPreg::test_preg_match_all() {
  Variant matches;

  f_preg_match_all("/\\(?  (\\d{3})?  \\)?  (?(1)  [\\-\\s] ) \\d{3}-\\d{4}/x",
                   "Call 555-1212 or 1-800-555-1212", ref(matches));
  VS(f_print_r(matches, true),
     "Array\n"
     "(\n"
     "    [0] => Array\n"
     "        (\n"
     "            [0] => 555-1212\n"
     "            [1] => 800-555-1212\n"
     "        )\n"
     "\n"
     "    [1] => Array\n"
     "        (\n"
     "            [0] => \n"
     "            [1] => 800\n"
     "        )\n"
     "\n"
     ")\n");

  // The \\2 is an example of backreferencing. This tells pcre that
  // it must match the second set of parentheses in the regular expression
  // itself, which would be the ([\w]+) in this case. The extra backslash is
  // required because the string is in double quotes.
  String html = "<b>bold text</b><a href=howdy.html>click me</a>";
  f_preg_match_all("/(<([\\w]+)[^>]*>)(.*)(<\\/\\2>)/", html, ref(matches),
                   k_PREG_SET_ORDER);
  VS(f_print_r(matches, true),
     "Array\n"
     "(\n"
     "    [0] => Array\n"
     "        (\n"
     "            [0] => <b>bold text</b>\n"
     "            [1] => <b>\n"
     "            [2] => b\n"
     "            [3] => bold text\n"
     "            [4] => </b>\n"
     "        )\n"
     "\n"
     "    [1] => Array\n"
     "        (\n"
     "            [0] => <a href=howdy.html>click me</a>\n"
     "            [1] => <a href=howdy.html>\n"
     "            [2] => a\n"
     "            [3] => click me\n"
     "            [4] => </a>\n"
     "        )\n"
     "\n"
     ")\n");

  String str = "a: 1\nb: 2\nc: 3\n";
  f_preg_match_all("/(?<name>\\w+): (?<digit>\\d+)/", str, ref(matches));
  VS(f_print_r(matches, true),
     "Array\n"
     "(\n"
     "    [0] => Array\n"
     "        (\n"
     "            [0] => a: 1\n"
     "            [1] => b: 2\n"
     "            [2] => c: 3\n"
     "        )\n"
     "\n"
     "    [name] => Array\n"
     "        (\n"
     "            [0] => a\n"
     "            [1] => b\n"
     "            [2] => c\n"
     "        )\n"
     "\n"
     "    [1] => Array\n"
     "        (\n"
     "            [0] => a\n"
     "            [1] => b\n"
     "            [2] => c\n"
     "        )\n"
     "\n"
     "    [digit] => Array\n"
     "        (\n"
     "            [0] => 1\n"
     "            [1] => 2\n"
     "            [2] => 3\n"
     "        )\n"
     "\n"
     "    [2] => Array\n"
     "        (\n"
     "            [0] => 1\n"
     "            [1] => 2\n"
     "            [2] => 3\n"
     "        )\n"
     "\n"
     ")\n");

  return Count(true);
}

bool TestExtPreg::test_preg_replace() {
  {
    String str = "April 15, 2003";
    String pattern = "/(\\w+) (\\d+), (\\d+)/i";
    String replacement = "${1}1,$3";
    VS(f_preg_replace(pattern, replacement, str), "April1,2003");
  }
  {
    String str = "The quick brown fox jumped over the lazy dog.";
    Variant patterns, replacements;
    patterns.set(0, "/quick/");
    patterns.set(1, "/brown/");
    patterns.set(2, "/fox/");
    replacements.set(2, "bear");
    replacements.set(1, "black");
    replacements.set(0, "slow");
    VS(f_preg_replace(patterns, replacements, str),
       "The bear black slow jumped over the lazy dog.");

    f_ksort(ref(patterns));
    f_ksort(ref(replacements));
    VS(f_preg_replace(patterns, replacements, str),
       "The slow black bear jumped over the lazy dog.");
  }
  {
    Array patterns = CREATE_VECTOR2("/(19|20)(\\d{2})-(\\d{1,2})-(\\d{1,2})/",
                                    "/^\\s*{(\\w+)}\\s*=/");
    Array replace = CREATE_VECTOR2("\\3/\\4/\\1\\2", "$\\1 =");
    VS(f_preg_replace(patterns, replace, "{startDate} = 1999-5-27"),
       "$startDate = 5/27/1999");
  }
  {
    String str = "foo   o";
    str = f_preg_replace("/\\s\\s+/", " ", str);
    VS(str, "foo o");
  }
  {
    Variant count = 0;
    f_preg_replace(CREATE_VECTOR2("/\\d/", "/\\s/"), "*", "xp 4 to", -1,
                   ref(count));
    VS(count, 3);
  }
  {
    String html_body = "<html><body></body></html>";
    String html_body2 = f_preg_replace("/(<\\/?\\w+[^>]*>)/e",
                               "strtoupper(\"$1\")",
                               html_body);
    VS(html_body2, "<HTML><BODY></BODY></HTML>");

    String css_text = "#AAAA;";
    String css_text2 = f_preg_replace("/#([A-Fa-f0-9]{3,6});/e",
                                      "strtolower(\"#\\1;\");", css_text);
    VS(css_text2, "#aaaa;");

    String rgb_text = "rgb(13, 14, 15)";
    String rgb_text2 =
      f_preg_replace("/rgb\\(([0-9]{1,3}), ([0-9]{1,3}), ([0-9]{1,3})\\)/e",
                     "sprintf(\"%02x%02x%02x\", \"\\1\", \"\\2\", \"\\3\")",
                     rgb_text);
    VS(rgb_text2, "0d0e0f");

    String res = f_preg_replace("/(a*)(b*)/e",
                                "test_preg_rep(\"\\1\",\"smu\\\"rf\",\"\\2\")",
                                "aaabbbblahblahaabbbababab");
    VS(res, "BBBBaaalahBlahBBBaaBaBaBa");

    try {
      f_preg_replace("/(<\\/?)(\\w+)([^>]*>)/e",
                     "'\\\\1'.strtoupper('\\\\2').'\\\\3'",
                     html_body);
    } catch (NotSupportedException e) {
      return Count(true);
    }
  }
  return Count(false);
}

bool TestExtPreg::test_preg_replace_callback() {
  {
    // this text was used in 2002
    // we want to get this up to date for 2003
    String text = "April fools day is 04/01/2002\n"
      "Last christmas was 12/24/2001\n";
    text = f_preg_replace_callback("|(\\d{2}/\\d{2}/)(\\d{4})|", "next_year",
                                   text);
    VS(text, "April fools day is 04/01/2003\nLast christmas was 12/24/2002\n");
  }
  return Count(true);
}

bool TestExtPreg::test_preg_split() {
  // split the phrase by any number of commas or space characters,
  // which include " ", \r, \t, \n and \f
  {
    Array keywords = f_preg_split("/[\\s,]+/",
                                  "hypertext language, programming");
    VS(keywords.size(), 3);
    VS(keywords[0], "hypertext");
    VS(keywords[1], "language");
    VS(keywords[2], "programming");
  }
  {
    String str = "string";
    Array chars = f_preg_split("//", str, -1, k_PREG_SPLIT_NO_EMPTY);
    VS(chars.size(), 6);
    VS(chars[0], "s");
    VS(chars[1], "t");
    VS(chars[2], "r");
    VS(chars[3], "i");
    VS(chars[4], "n");
    VS(chars[5], "g");
  }
  {
    String str = "hypertext language programming";
    Array chars = f_preg_split("/ /", str, -1, k_PREG_SPLIT_OFFSET_CAPTURE);
    VS(f_print_r(chars, true),
       "Array\n"
       "(\n"
       "    [0] => Array\n"
       "        (\n"
       "            [0] => hypertext\n"
       "            [1] => 0\n"
       "        )\n"
       "\n"
       "    [1] => Array\n"
       "        (\n"
       "            [0] => language\n"
       "            [1] => 10\n"
       "        )\n"
       "\n"
       "    [2] => Array\n"
       "        (\n"
       "            [0] => programming\n"
       "            [1] => 19\n"
       "        )\n"
       "\n"
       ")\n");
  }
  return Count(true);
}

bool TestExtPreg::test_preg_quote() {
  {
    String keywords = "$40 for a g3/400";
    keywords = f_preg_quote(keywords, "/");
    VS(keywords, "\\$40 for a g3\\/400");
  }

  // In this example, preg_quote($word) is used to keep the
  // asterisks from having special meaning to the regular
  // expression.
  {
    String textbody = "This book is *very* difficult to find.";
    String word = "*very*";
    textbody = f_preg_replace(String("/") + f_preg_quote(word) + "/",
                              String("<i>") + word + "</i>",
                              textbody);
    VS(textbody, "This book is <i>*very*</i> difficult to find.");
  }
  return Count(true);
}

bool TestExtPreg::test_preg_last_error() {
  f_preg_match("/(?:\\D+|<\\d+>)*[!?]/", "foobar foobar foobar");
  VS(f_preg_last_error(), 2);
  return Count(true);
}

bool TestExtPreg::test_ereg_replace() {
  {
    String str = "This is a test";
    VS(f_str_replace(" is", " was", str), "This was a test");
    VS(f_ereg_replace("( )is", "\\1was", str), "This was a test");
    VS(f_ereg_replace("(( )is)", "\\2was", str), "This was a test");
  }
  {
    int num = 4;
    String str = "This string has four words.";
    str = f_ereg_replace("four", num, str);
    VS(str, "This string has 4 words.");
  }
  {
    String test = "http://test.com/test";
    test = f_ereg_replace("[[:alpha:]]+://[^<>[:space:]]+[[:alnum:]/]",
                          "<a href=\"\\0\">\\0</a>", test);
    VS(test, "<a href=\"http://test.com/test\">http://test.com/test</a>");
  }
  return Count(true);
}

bool TestExtPreg::test_eregi_replace() {
  String pattern = "(>[^<]*)(suffix)";
  String replacement = "\\1<span class=\"search\">\\2</span>";
  String body = ">whateversuffix";
  body = f_eregi_replace(pattern, replacement, body);
  VS(body, ">whatever<span class=\"search\">suffix</span>");
  return Count(true);
}

bool TestExtPreg::test_ereg() {
  Variant regs;
  String date = "1973-04-30";
  VERIFY(f_ereg("([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})", date, ref(regs)));
  VS(regs[3], "30");
  VS(regs[2], "04");
  VS(regs[1], "1973");
  VS(regs[0], "1973-04-30");
  return Count(true);
}

bool TestExtPreg::test_eregi() {
  String str = "XYZ";
  VERIFY(f_eregi("z", str));
  return Count(true);
}

bool TestExtPreg::test_split() {
  {
    String mb = "\xe4\xbf\xa1\xe6\x81\xaf\x01  2366797";
    Array ret = f_split("\x01", mb);
    VS(ret[0], "\xe4\xbf\xa1\xe6\x81\xaf");
    VS(ret[1], "  2366797");
  }

  String date = "04/30/1973";
  Array ret = f_split("[/.-]", date);
  VS(ret[0], "04");
  VS(ret[1], "30");
  VS(ret[2], "1973");
  return Count(true);
}

bool TestExtPreg::test_spliti() {
  String str = "aBBBaCCCADDDaEEEaGGGA";
  Array chunks = f_spliti("a", str, 5);
  VS(chunks[0], "");
  VS(chunks[1], "BBB");
  VS(chunks[2], "CCC");
  VS(chunks[3], "DDD");
  VS(chunks[4], "EEEaGGGA");
  return Count(true);
}

bool TestExtPreg::test_sql_regcase() {
  VS(f_sql_regcase("Foo - bar."), "[Ff][Oo][Oo] - [Bb][Aa][Rr].");
  return Count(true);
}

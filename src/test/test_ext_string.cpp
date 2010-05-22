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

#include <test/test_ext_string.h>
#include <runtime/ext/ext_string.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtString::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_addcslashes);
  RUN_TEST(test_stripcslashes);
  RUN_TEST(test_addslashes);
  RUN_TEST(test_stripslashes);
  RUN_TEST(test_bin2hex);
  RUN_TEST(test_nl2br);
  RUN_TEST(test_quotemeta);
  RUN_TEST(test_str_shuffle);
  RUN_TEST(test_strrev);
  RUN_TEST(test_strtolower);
  RUN_TEST(test_strtoupper);
  RUN_TEST(test_ucfirst);
  RUN_TEST(test_ucwords);
  RUN_TEST(test_strip_tags);
  RUN_TEST(test_trim);
  RUN_TEST(test_ltrim);
  RUN_TEST(test_rtrim);
  RUN_TEST(test_chop);
  RUN_TEST(test_explode);
  RUN_TEST(test_implode);
  RUN_TEST(test_join);
  RUN_TEST(test_str_split);
  RUN_TEST(test_chunk_split);
  RUN_TEST(test_strtok);
  RUN_TEST(test_str_replace);
  RUN_TEST(test_str_ireplace);
  RUN_TEST(test_substr_replace);
  RUN_TEST(test_substr);
  RUN_TEST(test_str_pad);
  RUN_TEST(test_str_repeat);
  RUN_TEST(test_wordwrap);
  RUN_TEST(test_html_entity_decode);
  RUN_TEST(test_htmlentities);
  RUN_TEST(test_htmlspecialchars_decode);
  RUN_TEST(test_htmlspecialchars);
  RUN_TEST(test_quoted_printable_encode);
  RUN_TEST(test_quoted_printable_decode);
  RUN_TEST(test_convert_uudecode);
  RUN_TEST(test_convert_uuencode);
  RUN_TEST(test_str_rot13);
  RUN_TEST(test_crc32);
  RUN_TEST(test_crypt);
  RUN_TEST(test_md5);
  RUN_TEST(test_sha1);
  RUN_TEST(test_strtr);
  RUN_TEST(test_convert_cyr_string);
  RUN_TEST(test_get_html_translation_table);
  RUN_TEST(test_hebrev);
  RUN_TEST(test_hebrevc);
  RUN_TEST(test_setlocale);
  RUN_TEST(test_localeconv);
  RUN_TEST(test_nl_langinfo);
  RUN_TEST(test_echo);
  RUN_TEST(test_print);
  RUN_TEST(test_printf);
  RUN_TEST(test_vprintf);
  RUN_TEST(test_sprintf);
  RUN_TEST(test_vsprintf);
  RUN_TEST(test_sscanf);
  RUN_TEST(test_chr);
  RUN_TEST(test_ord);
  RUN_TEST(test_money_format);
  RUN_TEST(test_number_format);
  RUN_TEST(test_strcmp);
  RUN_TEST(test_strncmp);
  RUN_TEST(test_strnatcmp);
  RUN_TEST(test_strcasecmp);
  RUN_TEST(test_strncasecmp);
  RUN_TEST(test_strnatcasecmp);
  RUN_TEST(test_strcoll);
  RUN_TEST(test_substr_compare);
  RUN_TEST(test_strchr);
  RUN_TEST(test_strrchr);
  RUN_TEST(test_strstr);
  RUN_TEST(test_stristr);
  RUN_TEST(test_strpbrk);
  RUN_TEST(test_strpos);
  RUN_TEST(test_stripos);
  RUN_TEST(test_strrpos);
  RUN_TEST(test_strripos);
  RUN_TEST(test_substr_count);
  RUN_TEST(test_strspn);
  RUN_TEST(test_strcspn);
  RUN_TEST(test_strlen);
  RUN_TEST(test_count_chars);
  RUN_TEST(test_str_word_count);
  RUN_TEST(test_levenshtein);
  RUN_TEST(test_similar_text);
  RUN_TEST(test_soundex);
  RUN_TEST(test_metaphone);
  RUN_TEST(test_parse_str);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtString::test_addcslashes() {
  VS(f_addcslashes("ABCDEFGH\n", "A..D\n"), "\\A\\B\\C\\DEFGH\\n");
  VS(f_addcslashes(String("\x00\x0D\n", 3, AttachLiteral), null_string),
     "\\000\\r\\n");
  return Count(true);
}

bool TestExtString::test_stripcslashes() {
  VS(f_stripcslashes("\\A\\B\\C\\DEFGH\\n"), "ABCDEFGH\n");
  return Count(true);
}

bool TestExtString::test_addslashes() {
  VS(f_addslashes("'\"\\\n"), "\\'\\\"\\\\\n");
  return Count(true);
}

bool TestExtString::test_stripslashes() {
  VS(f_stripslashes("\\'\\\"\\\\\n"), "'\"\\\n");
  return Count(true);
}

bool TestExtString::test_bin2hex() {
  VS(f_bin2hex("ABC\n"), "4142430a");
  return Count(true);
}

bool TestExtString::test_nl2br() {
  VS(f_nl2br("A\nB"), "A<br />\nB");
  return Count(true);
}

bool TestExtString::test_quotemeta() {
  VS(f_quotemeta(". \\ + * ? [ ^ ] ( $ )"),
     "\\. \\\\ \\+ \\* \\? \\[ \\^ \\] \\( \\$ \\)");
  return Count(true);
}

bool TestExtString::test_str_shuffle() {
  VERIFY(f_str_shuffle("ABC").size() == 3);
  return Count(true);
}

bool TestExtString::test_strrev() {
  VS(f_strrev("ABC"), "CBA");
  return Count(true);
}

bool TestExtString::test_strtolower() {
  VS(f_strtolower("ABC"), "abc");
  return Count(true);
}

bool TestExtString::test_strtoupper() {
  VS(f_strtoupper("abc"), "ABC");
  return Count(true);
}

bool TestExtString::test_ucfirst() {
  VS(f_ucfirst("abc"), "Abc");
  return Count(true);
}

bool TestExtString::test_ucwords() {
  VS(f_ucwords("abc def"), "Abc Def");
  return Count(true);
}

bool TestExtString::test_strip_tags() {
  String text = "<p>Test paragraph.</p><!-- Comment --> "
    "<a href=\"#fragment\">Other text</a>";
  VS(f_strip_tags(text), "Test paragraph. Other text");
  VS(f_strip_tags(text, "<p><a>"),
     "<p>Test paragraph.</p> <a href=\"#fragment\">Other text</a>");
  return Count(true);
}

bool TestExtString::test_trim() {
  VS(f_trim(" abc "), "abc");
  return Count(true);
}

bool TestExtString::test_ltrim() {
  VS(f_ltrim(" abc "), "abc ");
  return Count(true);
}

bool TestExtString::test_rtrim() {
  VS(f_rtrim(" abc "), " abc");
  return Count(true);
}

bool TestExtString::test_chop() {
  VS(f_chop(" abc "), " abc");
  return Count(true);
}

bool TestExtString::test_explode() {
  {
    String pizza = "piece1 piece2 piece3 piece4 piece5 piece6";
    Array pieces = f_explode(" ", pizza);
    VS(pieces[0], "piece1");
    VS(pieces[1], "piece2");
  }
  {
    String str = "one|two|three|four";
    Array ret = f_explode("|", str, 2);
    VS(ret[0], "one");
    VS(ret[1], "two|three|four");
  }
  {
    String str = "one|two|three|four";
    Array ret = f_explode("|", str, -1);
    VERIFY(ret.size() == 3);
    VS(ret[0], "one");
    VS(ret[1], "two");
    VS(ret[2], "three");
  }
  {
    String str = "ab";
    Array ret = f_explode("b", str);
    VS(ret[0], "a");
    VS(ret[1], "");
  }
  {
    String str = "b";
    Array ret = f_explode("b", str);
    VS(ret[0], "");
    VS(ret[1], "");
  }
  {
    String str = "bb";
    Array ret = f_explode("b", str);
    VS(ret[0], "");
    VS(ret[1], "");
    VS(ret[2], "");
  }
  {
    String str = "";
    Array ret = f_explode("b", str);
    VS(ret[0], "");
  }
  return Count(true);
}

bool TestExtString::test_implode() {
  {
    Array arr = CREATE_VECTOR3("lastname", "email", "phone");
    VS(f_implode(",", arr), "lastname,email,phone");
  }
  {
    Array arr = CREATE_VECTOR3("lastname", "", "phone");
    VS(f_implode(",", arr), "lastname,,phone");
  }
  {
    Array arr = CREATE_VECTOR3("", "email", "phone");
    VS(f_implode(",", arr), ",email,phone");
  }
  {
    Array arr = CREATE_VECTOR3("", "", "");
    VS(f_implode(",", arr), ",,");
  }
  {
    Array arr = Array::Create();
    VS(f_implode(",", arr), "");
  }
  return Count(true);
}

bool TestExtString::test_join() {
  Array arr = CREATE_VECTOR3("lastname", "email", "phone");
  VS(f_join(",", arr), "lastname,email,phone");
  return Count(true);
}

bool TestExtString::test_str_split() {
  String str = "Hello Friend";
  Array arr1 = f_str_split(str);
  VERIFY(arr1.size() == str.size());
  VS(arr1[0], "H");
  VS(arr1[1], "e");

  Array arr2 = f_str_split(str, 3);
  VERIFY(arr2.size() == 4);
  VS(arr2[0], "Hel");
  VS(arr2[1], "lo ");
  return Count(true);
}

bool TestExtString::test_chunk_split() {
  String ret = f_chunk_split("ABCD", 2);
  VS(ret, "AB\r\nCD\r\n");
  return Count(true);
}

bool TestExtString::test_strtok() {
  String s = "This is\tan ";
  Array tokens;
  Variant tok = f_strtok(s, " \n\t");
  while (tok) {
    tokens.append(tok);
    tok = f_strtok(" \n\t");
  }
  VS(tokens, CREATE_VECTOR3("This", "is", "an"));
  return Count(true);
}

bool TestExtString::test_str_replace() {
  {
    VS(f_str_replace("%body%", "black", "<body text='%body%'>"),
       "<body text='black'>");
  }
  {
    Array vowels;
    vowels.append("a");
    vowels.append("e");
    vowels.append("i");
    vowels.append("o");
    vowels.append("u");
    vowels.append("A");
    vowels.append("E");
    vowels.append("I");
    vowels.append("O");
    vowels.append("U");
    VS(f_str_replace(vowels, "", "Hello World of PHP"), "Hll Wrld f PHP");
  }
  {
    String phrase  = "You should eat fruits, vegetables, and fiber every day.";
    Array healthy = CREATE_VECTOR3("fruits", "vegetables", "fiber");
    Array yummy   = CREATE_VECTOR3("pizza", "beer", "ice cream");
    VS(f_str_replace(healthy, yummy, phrase),
       "You should eat pizza, beer, and ice cream every day.");
  }
  {
    Variant count;
    Variant str = f_str_replace("ll", "", "good golly miss molly!",
                                ref(count));
    VS(count, 2);
  }
  {
    Array letters = CREATE_VECTOR2("a", "p");
    Array fruit = CREATE_VECTOR2("apple", "pear");
    String text = "a p";
    VS(f_str_replace(letters, fruit, text), "apearpearle pear");
  }

  return Count(true);
}

bool TestExtString::test_str_ireplace() {
  VS(f_str_ireplace("%body%", "black", "<body text='%BODY%'>"),
     "<body text='black'>");
  VS(f_str_ireplace("%body%", "Black", "<body Text='%BODY%'>"),
     "<body Text='Black'>");
  return Count(true);
}

bool TestExtString::test_substr_replace() {
  String var = "ABCDEFGH:/MNRPQR/";
  VS(f_substr_replace(var, "bob", 0), "bob");
  VS(f_substr_replace(var, "bob", 0, f_strlen(var)), "bob");
  VS(f_substr_replace(var, "bob", 0, 0), "bobABCDEFGH:/MNRPQR/");

  VS(f_substr_replace(var, "bob", 10, -1), "ABCDEFGH:/bob/");
  VS(f_substr_replace(var, "bob", -7, -1), "ABCDEFGH:/bob/");

  VS(f_substr_replace(var, "", 10, -1), "ABCDEFGH://");
  return Count(true);
}

bool TestExtString::test_substr() {
  VS(f_substr("abcdef", 1), "bcdef");
  VS(f_substr("abcdef", 1, 3), "bcd");
  VS(f_substr("abcdef", 0, 4), "abcd");
  VS(f_substr("abcdef", 0, 8), "abcdef");
  VS(f_substr("abcdef", -1, 1), "f");
  return Count(true);
}

bool TestExtString::test_str_pad() {
  String input = "Alien";
  VS(f_str_pad(input, 10), "Alien     ");
  VS(f_str_pad(input, 10, "-=", k_STR_PAD_LEFT), "-=-=-Alien");
  VS(f_str_pad(input, 10, "_", k_STR_PAD_BOTH), "__Alien___");
  VS(f_str_pad(input, 6 , "___"), "Alien_");
  VS(f_str_pad(input, 6 , String("\0", 1, AttachLiteral)),
     String("Alien\0", 6, AttachLiteral));
  return Count(true);
}

bool TestExtString::test_str_repeat() {
  VS(f_str_repeat("-=", 10), "-=-=-=-=-=-=-=-=-=-=");
  return Count(true);
}

bool TestExtString::test_wordwrap() {
  {
    String text = "The quick brown fox jumped over the lazy dog.";
    VS(f_wordwrap(text, 20, "<br />\n"),
       "The quick brown fox<br />\njumped over the lazy<br />\ndog.");
  }
  {
    String text = "A very long woooooooooooord.";
    VS(f_wordwrap(text, 8, "\n", true), "A very\nlong\nwooooooo\nooooord.");
  }
  return Count(true);
}

bool TestExtString::test_html_entity_decode() {
  String orig = "I'll \"walk\" the <b>dog</b> now";
  String a = f_htmlentities(orig);
  VS(a, "I'll &quot;walk&quot; the &lt;b&gt;dog&lt;/b&gt; now");
  VS(f_html_entity_decode(a), orig);
  return Count(true);
}

bool TestExtString::test_htmlentities() {
  String str = "A 'quote' is <b>bold</b>";
  VS(f_htmlentities(str), "A 'quote' is &lt;b&gt;bold&lt;/b&gt;");
  VS(f_htmlentities(str, k_ENT_QUOTES),
     "A &#039;quote&#039; is &lt;b&gt;bold&lt;/b&gt;");
  return Count(true);
}

bool TestExtString::test_htmlspecialchars_decode() {
  String str = "<p>this -&gt; &quot;</p>";
  VS(f_htmlspecialchars_decode(str), "<p>this -> \"</p>");
  return Count(true);
}

bool TestExtString::test_htmlspecialchars() {
  VS(f_htmlspecialchars("<a href='test'>Test</a>", k_ENT_QUOTES),
     "&lt;a href=&#039;test&#039;&gt;Test&lt;/a&gt;");
  return Count(true);
}

bool TestExtString::test_quoted_printable_encode() {
  VS(f_quoted_printable_encode("egfe \015\t"), "egfe=20=0D=09");
  return Count(true);
}

bool TestExtString::test_quoted_printable_decode() {
  VS(f_quoted_printable_decode("=65=67=66=65="), "egfe");
  return Count(true);
}

bool TestExtString::test_convert_uudecode() {
  VS(f_convert_uudecode("+22!L;W9E(%!(4\"$`\n`"), "I love PHP!");
  return Count(true);
}

bool TestExtString::test_convert_uuencode() {
  VS(f_convert_uuencode("test\ntext text\r\n"),
     "0=&5S=`IT97AT('1E>'0-\"@``\n`\n");
  return Count(true);
}

bool TestExtString::test_str_rot13() {
  VS(f_str_rot13("PHP 4.3.0"), "CUC 4.3.0");
  return Count(true);
}

bool TestExtString::test_crc32() {
  VS(f_crc32("The quick brown fox jumped over the lazy dog."), 2191738434LL);
  return Count(true);
}

bool TestExtString::test_crypt() {
  VERIFY(!f_crypt("mypassword").empty());
  return Count(true);
}

bool TestExtString::test_md5() {
  VS(f_md5("apple"), "1f3870be274f6c49b3e31a0c6728957f");
  return Count(true);
}

bool TestExtString::test_sha1() {
  VS(f_sha1("apple"), "d0be2dc421be4fcd0172e5afceea3970e2f3d940");
  return Count(true);
}

bool TestExtString::test_strtr() {
  Array trans = CREATE_MAP2("hello", "hi", "hi", "hello");
  VS(f_strtr("hi all, I said hello", trans), "hello all, I said hi");
  return Count(true);
}

bool TestExtString::test_convert_cyr_string() {
  VS(f_convert_cyr_string("abc", "a", "d"), "abc"); // sanity
  return Count(true);
}

bool TestExtString::test_get_html_translation_table() {
  VERIFY(!f_get_html_translation_table(k_HTML_ENTITIES).empty());
  return Count(true);
}

bool TestExtString::test_hebrev() {
  VS(f_hebrev("test"), "test"); // sanity
  return Count(true);
}

bool TestExtString::test_hebrevc() {
  VS(f_hebrevc("test"), "test"); // sanity
  return Count(true);
}

bool TestExtString::test_setlocale() {
  VERIFY(!f_setlocale(0, k_LC_ALL, 0).toString().empty());
  return Count(true);
}

bool TestExtString::test_localeconv() {
  VERIFY(!f_localeconv().empty());
  return Count(true);
}

bool TestExtString::test_nl_langinfo() {
#ifndef MAC_OS_X
  VS(f_nl_langinfo(k_AM_STR), "AM");
#endif
  return Count(true);
}

bool TestExtString::test_echo() {
  g_context->obStart();
  f_echo(0, "test");
  String output = g_context->obGetContents();
  g_context->obEnd();
  VS(output, "test");
  return Count(true);
}

bool TestExtString::test_print() {
  g_context->obStart();
  f_print("test");
  String output = g_context->obGetContents();
  g_context->obEnd();
  VS(output, "test");
  return Count(true);
}

bool TestExtString::test_printf() {
  g_context->obStart();
  f_printf(2, "A%sB%dC", CREATE_VECTOR2("test", 10));
  String output = g_context->obGetContents();
  g_context->obEnd();
  VS(output, "AtestB10C");
  return Count(true);
}

bool TestExtString::test_vprintf() {
  g_context->obStart();
  f_vprintf("A%sB%dC", CREATE_VECTOR2("test", 10));
  String output = g_context->obGetContents();
  g_context->obEnd();
  VS(output, "AtestB10C");
  return Count(true);
}

bool TestExtString::test_sprintf() {
  VS(f_sprintf(2, "A%sB%dC", CREATE_VECTOR2("test", 10)), "AtestB10C");
  VS(f_sprintf(2, "%010s", CREATE_VECTOR1("1101")), "0000001101");
  VS(f_sprintf(2, "%02d", CREATE_VECTOR1("09")), "09");
  return Count(true);
}

bool TestExtString::test_vsprintf() {
  VS(f_vsprintf("A%sB%dC", CREATE_VECTOR2("test", 10)), "AtestB10C");
  return Count(true);
}

bool TestExtString::test_sscanf() {
  VS(f_sscanf(0, "SN/2350001", "SN/%d"), CREATE_VECTOR1(2350001));

  Variant out;
  VS(f_sscanf(0, "SN/2350001", "SN/%d", CREATE_VECTOR1(ref(out))), 1);
  VS(out, 2350001);
  return Count(true);
}

bool TestExtString::test_chr() {
  VS(f_chr(92), "\\");
  return Count(true);
}

bool TestExtString::test_ord() {
  VS(f_ord("\\"), 92);
  return Count(true);
}

bool TestExtString::test_money_format() {
  VS(f_money_format("%i", 1234.56), "1234.56");
  return Count(true);
}

bool TestExtString::test_number_format() {
  VS(f_number_format(1234.56), "1,235");
  return Count(true);
}

bool TestExtString::test_strcmp() {
  VERIFY(f_strcmp("a", "b") < 0);
  VERIFY(f_strcmp("a", "A") > 0);
  return Count(true);
}

bool TestExtString::test_strncmp() {
  VERIFY(f_strncmp("a", "ab", 1) == 0);
  return Count(true);
}

bool TestExtString::test_strnatcmp() {
  VERIFY(f_strnatcmp("a", "b") < 0);
  return Count(true);
}

bool TestExtString::test_strcasecmp() {
  VERIFY(f_strcasecmp("a", "A") == 0);
  return Count(true);
}

bool TestExtString::test_strncasecmp() {
  VERIFY(f_strncasecmp("a", "Ab", 1) == 0);
  return Count(true);
}

bool TestExtString::test_strnatcasecmp() {
  VERIFY(f_strnatcasecmp("a", "Ab") < 0);
  return Count(true);
}

bool TestExtString::test_strcoll() {
  VERIFY(f_strcoll("a", "b") < 0);
  VERIFY(f_strcoll("a", "A") > 0);
  return Count(true);
}

bool TestExtString::test_substr_compare() {
  VS(f_substr_compare("abcde", "bc", 1, 2), 0);
  VS(f_substr_compare("abcde", "de", -2, 2), 0);
  VS(f_substr_compare("abcde", "bcg", 1, 2), 0);
  VS(f_substr_compare("abcde", "BC", 1, 2, true), 0);
  VS(f_substr_compare("abcde", "bc", 1, 3), 1);
  VS(f_substr_compare("abcde", "cd", 1, 2), -1);
  return Count(true);
}

bool TestExtString::test_strchr() {
  String email  = "name@example.com";
  VS(f_strchr(email, "@"), "@example.com");
  return Count(true);
}

bool TestExtString::test_strrchr() {
  String text = "Line 1\nLine 2\nLine 3";
  VS(f_strrchr(text, 10), "\nLine 3");
  return Count(true);
}

bool TestExtString::test_strstr() {
  String email  = "name@example.com";
  VS(f_strstr(email, "@"), "@example.com");
  return Count(true);
}

bool TestExtString::test_stristr() {
  VS(f_stristr("Hello World!", "earth"), false);
  VS(f_stristr("APPLE", 97), "APPLE");
  return Count(true);
}

bool TestExtString::test_strpbrk() {
  String text = "This is a Simple text.";
  VS(f_strpbrk(text, "mi"), "is is a Simple text.");
  VS(f_strpbrk(text, "S"), "Simple text.");
  return Count(true);
}

bool TestExtString::test_strpos() {
  VS(f_strpos("abcdef abcdef", "a"), 0);
  VS(f_strpos("abcdef abcdef", "a", 1), 7);
  VS(f_strpos("abcdef abcdef", "A", 1), false);
  VS(f_strpos("abcdef abcdef", "", 0), false);
  return Count(true);
}

bool TestExtString::test_stripos() {
  VS(f_stripos("abcdef abcdef", "A", 1), 7);
  return Count(true);
}

bool TestExtString::test_strrpos() {
  VS(f_strrpos("abcdef abcdef", "a"), 7);
  return Count(true);
}

bool TestExtString::test_strripos() {
  VS(f_strripos("abcdef abcdef", "A"), 7);
  return Count(true);
}

bool TestExtString::test_substr_count() {
  String text = "This is a test";
  VS(f_substr_count(text, "is"), 2);
  VS(f_substr_count(text, "is", 3), 1);
  VS(f_substr_count(text, "is", 3, 3), 0);
  VS(f_substr_count("gcdgcdgcd", "gcdgcd"), 1);
  return Count(true);
}

bool TestExtString::test_strspn() {
  VS(f_strspn("foo", "o", 1, 2), 2);
  return Count(true);
}

bool TestExtString::test_strcspn() {
  VS(f_strcspn("foo", "o", 1, 2), 0);
  return Count(true);
}

bool TestExtString::test_strlen() {
  VS(f_strlen("test"), 4);
  return Count(true);
}

bool TestExtString::test_count_chars() {
  Array ret = f_count_chars("Two Ts and one F.");
  VS(ret[f_ord("T")], 2);
  return Count(true);
}

bool TestExtString::test_str_word_count() {
  VS(f_str_word_count("Two Ts and one F."), 5);
  return Count(true);
}

bool TestExtString::test_levenshtein() {
  VS(f_levenshtein("carrrot", "carrot"), 1);
  return Count(true);
}

bool TestExtString::test_similar_text() {
  VS(f_similar_text("carrrot", "carrot"), 6);
  return Count(true);
}

bool TestExtString::test_soundex() {
  VS(f_soundex("carrot"), "C630");
  return Count(true);
}

bool TestExtString::test_metaphone() {
  VS(f_metaphone("carrot"), "KRT");
  return Count(true);
}

bool TestExtString::test_parse_str() {
  {
    Variant output;
    f_parse_str("first=value&arr[]=foo+bar&arr[]=baz", ref(output));
    VS(output["first"], "value");
    VS(output["arr"][0], "foo bar");
    VS(output["arr"][1], "baz");
  }
  {
    Variant output;
    f_parse_str("a[2][i]=3&a[4][i]=5", ref(output));
    VS(output["a"][2]["i"], "3");
    VS(output["a"][4]["i"], "5");
  }

  return Count(true);
}

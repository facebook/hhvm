<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x, true); }

//////////////////////////////////////////////////////////////////////

function test_preg_rep($a,$b,$c) {
  return strtoupper($c).$a;
}

function test_preg_grep() {
  $array = array("foo 123.1", "fg 24bar", "123.1", "24");
  $fl_array = preg_grep("/^(\\d+)?\\.\\d+$/", $array);
  VS(count(fl_array), 1);
  VS($fl_array[2], "123.1");

  VS(preg_grep("/a/", array("c", "b")), array());
}

function test_preg_match() {
  // The "i" after the pattern delimiter indicates a case-insensitive search
  VS(preg_match("/php/i", "PHP is a scripting language."), 1);

  // The \b in the pattern indicates a word boundary, so only the distinct
  // word "web" is matched, and not a word partial like "webbing" or "cobweb"
  VS(preg_match("/\\bweb\\b/i", "is the web scripting"), 1);

  // get host name from URL
  $matches = null;
  preg_match("@^(?:http://)?([^/]+)@i",
             "http://www.php.net/index.html", $matches);
  $host = $matches[1];
  VS($host, "www.php.net");

  // get last two segments of host name
  preg_match("/[^.]+\\.[^.]+$/", $host, $matches);
  VS($matches[0], "php.net");

  $str = "foobar: 2008";
  preg_match("/(?<name>\\w+): (?<digit>\\d+)/", $str, $matches);
  VS(print_r($matches, true),
     "Array\n".
     "(\n".
     "    [0] => foobar: 2008\n".
     "    [name] => foobar\n".
     "    [1] => foobar\n".
     "    [digit] => 2008\n".
     "    [2] => 2008\n".
     ")\n");
}


function test_preg_match_all() {
  preg_match_all("/\\(?  (\\d{3})?  \\)?  (?(1)  [\\-\\s] ) \\d{3}-\\d{4}/x",
                   "Call 555-1212 or 1-800-555-1212", $matches);
  VS(print_r($matches, true),
     "Array\n".
     "(\n".
     "    [0] => Array\n".
     "        (\n".
     "            [0] => 555-1212\n".
     "            [1] => 800-555-1212\n".
     "        )\n".
     "\n".
     "    [1] => Array\n".
     "        (\n".
     "            [0] => \n".
     "            [1] => 800\n".
     "        )\n".
     "\n".
     ")\n");

  // The \\2 is an example of backreferencing. This tells pcre that
  // it must match the second set of parentheses in the regular expression
  // itself, which would be the ([\w]+) in this case. The extra backslash is
  // required because the string is in double quotes.
  $html = "<b>bold text</b><a href=howdy.html>click me</a>";
  preg_match_all("/(<([\\w]+)[^>]*>)(.*)(<\\/\\2>)/", $html, $matches,
                 PREG_SET_ORDER);
  VS(print_r($matches, true),
     "Array\n".
     "(\n".
     "    [0] => Array\n".
     "        (\n".
     "            [0] => <b>bold text</b>\n".
     "            [1] => <b>\n".
     "            [2] => b\n".
     "            [3] => bold text\n".
     "            [4] => </b>\n".
     "        )\n".
     "\n".
     "    [1] => Array\n".
     "        (\n".
     "            [0] => <a href=howdy.html>click me</a>\n".
     "            [1] => <a href=howdy.html>\n".
     "            [2] => a\n".
     "            [3] => click me\n".
     "            [4] => </a>\n".
     "        )\n".
     "\n".
     ")\n");

  $str = "a: 1\nb: 2\nc: 3\n";
  preg_match_all("/(?<name>\\w+): (?<digit>\\d+)/", $str, $matches);
  VS(print_r($matches, true),
     "Array\n".
     "(\n".
     "    [0] => Array\n".
     "        (\n".
     "            [0] => a: 1\n".
     "            [1] => b: 2\n".
     "            [2] => c: 3\n".
     "        )\n".
     "\n".
     "    [name] => Array\n".
     "        (\n".
     "            [0] => a\n".
     "            [1] => b\n".
     "            [2] => c\n".
     "        )\n".
     "\n".
     "    [1] => Array\n".
     "        (\n".
     "            [0] => a\n".
     "            [1] => b\n".
     "            [2] => c\n".
     "        )\n".
     "\n".
     "    [digit] => Array\n".
     "        (\n".
     "            [0] => 1\n".
     "            [1] => 2\n".
     "            [2] => 3\n".
     "        )\n".
     "\n".
     "    [2] => Array\n".
     "        (\n".
     "            [0] => 1\n".
     "            [1] => 2\n".
     "            [2] => 3\n".
     "        )\n".
     "\n".
     ")\n");
}

function test_preg_replace() {
  $str = "April 15, 2003";
  $pattern = "/(\\w+) (\\d+), (\\d+)/i";
  $replacement = "\${1}1,\$3";
  VS(preg_replace($pattern, $replacement, $str), "April1,2003");

  $str = "The quick brown fox jumped over the lazy dog.";
  $patterns = array();
  $replacements = array();
  $patterns[0] = "/quick/";
  $patterns[1] = "/brown/";
  $patterns[2] = "/fox/";
  $replacements[2] = "bear";
  $replacements[1] = "black";
  $replacements[0] = "slow";
  VS(preg_replace($patterns, $replacements, $str),
     "The bear black slow jumped over the lazy dog.");

  ksort($patterns);
  ksort($replacements);
  VS(preg_replace($patterns, $replacements, $str),
     "The slow black bear jumped over the lazy dog.");

  $foos = array();
  $foos[0] = "foo";
  $foos[1] = "Foo";
  $foos[2] = "FOO";
  $expFoo = array();
  $expFoo[0] = "FOO";
  $expFoo[1] = "FOO";
  $expFoo[2] = "FOO";
  VS(preg_replace("/some pattern/", "", array()), array());
  VS(preg_replace("/foo/i", "FOO", $foos), $expFoo);

  $patterns = array("/(19|20)(\\d{2})-(\\d{1,2})-(\\d{1,2})/",
                                  "/^\\s*{(\\w+)}\\s*=/");
  $replace = array("\\3/\\4/\\1\\2", "$\\1 =");
  VS(preg_replace($patterns, $replace, "{startDate} = 1999-5-27"),
     "\$startDate = 5/27/1999");

  $str = "foo   o";
  $str = preg_replace("/\\s\\s+/", " ", $str);
  VS($str, "foo o");

  $count = 0;
  preg_replace(array("/\\d/", "/\\s/"), "*", "xp 4 to", -1, $count);
  VS($count, 3);

  VS(preg_replace("/xxx", "w", "xxxx"), NULL);
  VS(preg_replace("/xxx/", "w", "xxxx"), "wx");
  VS(preg_replace("/xxy/", "w", "xxxx"), "xxxx");

  VS(preg_replace("/xxx", "w", array("xxxx")), array());
  VS(preg_replace("/xxx/", "w", array("xxxx")), array("wx"));
  VS(preg_replace("/xxx/", "w", array("xxxx", "yyyy")), array("wx", "yyyy"));
  VS(preg_replace(array("/xxx/", "/xxx"), "w", array("xxxx")), array());
  VS(preg_replace(array("/xxx/", "/xxx/"), "w", array("xxxx")), array("wx"));

  VS(preg_replace("/xxx", array("w"), array("xxxx")), false);
  VS(preg_replace(array("/xxx"), array("w"), array("xxxx")), array());
  VS(preg_replace(array("/xxx/"), array("w"), array("xxxx")), array("wx"));
}

function next_year($m) {
  return $m[1].((int)$m[2] + 1);
}

function test_preg_replace_callback() {
  $text = "April fools day is 04/01/2002\n".
    "Last christmas was 12/24/2001\n";
  $text = preg_replace_callback("|(\\d{2}/\\d{2}/)(\\d{4})|", "next_year",
                                $text);
  VS($text, "April fools day is 04/01/2003\nLast christmas was 12/24/2002\n");
}

function test_preg_split() {
  $keywords = preg_split("/[\\s,]+/",
                         "hypertext language, programming");
  VS(count($keywords), 3);
  VS($keywords[0], "hypertext");
  VS($keywords[1], "language");
  VS($keywords[2], "programming");

  $str = "string";
  $chars = preg_split("//", $str, -1, PREG_SPLIT_NO_EMPTY);
  VS(count($chars), 6);
  VS($chars[0], "s");
  VS($chars[1], "t");
  VS($chars[2], "r");
  VS($chars[3], "i");
  VS($chars[4], "n");
  VS($chars[5], "g");

  $str = "hypertext language programming";
  $chars = preg_split("/ /", $str, -1, PREG_SPLIT_OFFSET_CAPTURE);
  VS(print_r($chars, true),
     "Array\n".
     "(\n".
     "    [0] => Array\n".
     "        (\n".
     "            [0] => hypertext\n".
     "            [1] => 0\n".
     "        )\n".
     "\n".
     "    [1] => Array\n".
     "        (\n".
     "            [0] => language\n".
     "            [1] => 10\n".
     "        )\n".
     "\n".
     "    [2] => Array\n".
     "        (\n".
     "            [0] => programming\n".
     "            [1] => 19\n".
     "        )\n".
     "\n".
     ")\n");
}

function test_preg_quote() {
  $keywords = "$40 for a g3/400";
  $keywords = preg_quote($keywords, "/");
  VS($keywords, "\\$40 for a g3\\/400");

  // In this example, preg_quote($word) is used to keep the
  // asterisks from having special meaning to the regular
  // expression.
  $textbody = "This book is *very* difficult to find.";
  $word = "*very*";
  $textbody = preg_replace("/".preg_quote($word)."/",
                            "<i>". $word ."</i>",
                            $textbody);
  VS($textbody, "This book is <i>*very*</i> difficult to find.");
}

function test_ereg_replace() {
  $str = "This is a test";
  VS(str_replace(" is", " was", $str), "This was a test");
  VS(ereg_replace("( )is", "\\1was", $str), "This was a test");
  VS(ereg_replace("(( )is)", "\\2was", $str), "This was a test");

  $num = 4;
  $str = "This string has four words.";
  $str = ereg_replace("four", $num, $str);
  VS($str, "This string has 4 words.");

  $test = "http://test.com/test";
  $test = ereg_replace("[[:alpha:]]+://[^<>[:space:]]+[[:alnum:]/]",
                        "<a href=\"\\0\">\\0</a>", $test);
  VS($test, "<a href=\"http://test.com/test\">http://test.com/test</a>");
}

function test_eregi_replace() {
  $pattern = "(>[^<]*)(suffix)";
  $replacement = "\\1<span class=\"search\">\\2</span>";
  $body = ">whateversuffix";
  $body = eregi_replace($pattern, $replacement, $body);
  VS($body, ">whatever<span class=\"search\">suffix</span>");
}

function test_ereg() {
  $date = "1973-04-30";
  VERIFY(ereg("([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})", $date, $regs) !== false);
  VS($regs[3], "30");
  VS($regs[2], "04");
  VS($regs[1], "1973");
  VS($regs[0], "1973-04-30");
}

function test_eregi() {
  $str = "XYZ";
  VERIFY(eregi("z", $str) !== false);
}

function test_split() {
  $mb = "\xe4\xbf\xa1\xe6\x81\xaf\x01  2366797";
  $ret = split("\x01", $mb);
  VS($ret[0], "\xe4\xbf\xa1\xe6\x81\xaf");
  VS($ret[1], "  2366797");

  $date = "04/30/1973";
  $ret = split("[/.-]", $date);
  VS($ret[0], "04");
  VS($ret[1], "30");
  VS($ret[2], "1973");
}

function test_spliti() {
  $str = "aBBBaCCCADDDaEEEaGGGA";
  $chunks = spliti("a", $str, 5);
  VS($chunks[0], "");
  VS($chunks[1], "BBB");
  VS($chunks[2], "CCC");
  VS($chunks[3], "DDD");
  VS($chunks[4], "EEEaGGGA");
}

function test_sql_regcase() {
  VS(sql_regcase("Foo - bar."), "[Ff][Oo][Oo] - [Bb][Aa][Rr].");
}

test_preg_grep();
test_preg_match();
test_preg_match_all();
test_preg_replace();
test_preg_replace_callback();
test_preg_split();
test_preg_quote();
test_ereg_replace();
test_eregi_replace();
test_ereg();
test_eregi();
test_split();
test_spliti();
test_sql_regcase();

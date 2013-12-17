<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

VS(addcslashes("ABCDEFGH\n", "A..D\n"), "\\A\\B\\C\\DEFGH\\n");

VS(stripcslashes("\\A\\B\\C\\DEFGH\\n"), "ABCDEFGH\n");

VS(addslashes("'\"\\\n"), "\\'\\\"\\\\\n");

VS(stripslashes("\\'\\\"\\\\\n"), "'\"\\\n");

VS(bin2hex("ABC\n"), "4142430a");

VS(hex2bin("4142430a"), "ABC\n");

VS(nl2br("A\nB"), "A<br />\nB");
VS(nl2br("A\nB", true), "A<br />\nB");
VS(nl2br("A\nB", false), "A<br>\nB");

VS(quotemeta(". \\ + * ? [ ^ ] ( $ )"),
   "\\. \\\\ \\+ \\* \\? \\[ \\^ \\] \\( \\$ \\)");

VS(strlen(str_shuffle("ABC")), 3);

VS(strrev("ABC"), "CBA");

VS(strtolower("ABC"), "abc");

VS(strtoupper("abc"), "ABC");

VS(ucfirst("abc"), "Abc");

VS(lcfirst("ABC"), "aBC");

VS(ucwords("abc def"), "Abc Def");

$text = "<p>Test paragraph.</p><!-- Comment --> ".
  "<a href=\"#fragment\">Other text</a>";
VS(strip_tags($text), "Test paragraph. Other text");
VS(strip_tags($text, "<p><a>"),
   "<p>Test paragraph.</p> <a href=\"#fragment\">Other text</a>");

VS(trim(" abc "), "abc");

VS(ltrim(" abc "), "abc ");

VS(rtrim(" abc "), " abc");

VS(chop(" abc "), " abc");

{
  $metric = "create stuff";
  $pieces = explode(" ", $metric, 1);
  VS(count($pieces), 1);
  VS($pieces[0], "create stuff");
}
{
  $metric = "create stuff";
  $pieces = explode(" ", $metric, 0);
  VS(count($pieces), 1);
  VS($pieces[0], "create stuff");
}
{
  $pizza = "piece1 piece2 piece3 piece4 piece5 piece6";
  $pieces = explode(" ", $pizza);
  VS($pieces[0], "piece1");
  VS($pieces[1], "piece2");
}
{
  $str = "one|two|three|four";
  $ret = explode("|", $str, 2);
  VERIFY(count($ret) == 2);
  VS($ret[0], "one");
  VS($ret[1], "two|three|four");
}
{
  $str = "one|two|three|four";
  $ret = explode("|", $str, -1);
  VERIFY(count($ret) == 3);
  VS($ret[0], "one");
  VS($ret[1], "two");
  VS($ret[2], "three");
}
{
  $str = "ab";
  $ret = explode("b", $str);
  VS($ret[0], "a");
  VS($ret[1], "");
}
{
  $str = "b";
  $ret = explode("b", $str);
  VS($ret[0], "");
  VS($ret[1], "");
}
{
  $str = "bb";
  $ret = explode("b", $str);
  VS($ret[0], "");
  VS($ret[1], "");
  VS($ret[2], "");
}
{
  $str = "";
  $ret = explode("b", $str);
  VS($ret[0], "");
}

{
  $arr = array("lastname", "email", "phone");
  VS(implode(",", $arr), "lastname,email,phone");
}
{
  $arr = array("lastname", "", "phone");
  VS(implode(",", $arr), "lastname,,phone");
}
{
  $arr = array("", "email", "phone");
  VS(implode(",", $arr), ",email,phone");
}
{
  $arr = array("", "", "");
  VS(implode(",", $arr), ",,");
}
{
  $arr = array();
  VS(implode(",", $arr), "");
}

$arr = array("lastname", "email", "phone");
VS(join(",", $arr), "lastname,email,phone");

$str = "Hello Friend";
$arr1 = str_split($str);
VERIFY(count($arr1) == strlen($str));
VS($arr1[0], "H");
VS($arr1[1], "e");

$arr2 = str_split($str, 3);
VERIFY(count($arr2) == 4);
VS($arr2[0], "Hel");
VS($arr2[1], "lo ");

// TODO(#2512685): this behaves differently in the interpreter vs. jit
// $ret = chunk_split("ABCD", 2);
// VS($ret, "AB\r\nCD\r\n");

$s = "This is\tan ";
$tok = strtok($s, " \n\t");
while ($tok) {
  $tokens[] = $tok;
  $tok = strtok(" \n\t");
}
VS($tokens, array("This", "is", "an"));

{
  VS(str_replace("%body%", "black", "<body text='%body%'>"),
     "<body text='black'>");
}
{
  $vowels[] = "a";
  $vowels[] = "e";
  $vowels[] = "i";
  $vowels[] = "o";
  $vowels[] = "u";
  $vowels[] = "A";
  $vowels[] = "E";
  $vowels[] = "I";
  $vowels[] = "O";
  $vowels[] = "U";
  VS(str_replace($vowels, "", "Hello World of PHP"), "Hll Wrld f PHP");
}
{
  $phrase  = "You should eat fruits, vegetables, and fiber every day.";
  $healthy = array("fruits", "vegetables", "fiber");
  $yummy   = array("pizza", "beer", "ice cream");
  VS(str_replace($healthy, $yummy, $phrase),
     "You should eat pizza, beer, and ice cream every day.");
}
{
  $str = str_replace("ll", "", "good golly miss molly!",
                              $count);
  VS($count, 2);
}
{
  $letters = array("a", "p");
  $fruit = array("apple", "pear");
  $text = "a p";
  VS(str_replace($letters, $fruit, $text), "apearpearle pear");
}


VS(str_ireplace("%body%", "black", "<body text='%BODY%'>"),
   "<body text='black'>");
VS(str_ireplace("%body%", "Black", "<body Text='%BODY%'>"),
   "<body Text='Black'>");

$var = "ABCDEFGH:/MNRPQR/";
VS(substr_replace($var, "bob", 0), "bob");
VS(substr_replace($var, "bob", 0, strlen($var)), "bob");
VS(substr_replace($var, "bob", 0, 0), "bobABCDEFGH:/MNRPQR/");

VS(substr_replace($var, "bob", 10, -1), "ABCDEFGH:/bob/");
VS(substr_replace($var, "bob", -7, -1), "ABCDEFGH:/bob/");

VS(substr_replace($var, "", 10, -1), "ABCDEFGH://");

VS(substr("abcdef", 1), "bcdef");
VS(substr("abcdef", 1, 3), "bcd");
VS(substr("abcdef", 0, 4), "abcd");
VS(substr("abcdef", 0, 8), "abcdef");
VS(substr("abcdef", -1, 1), "f");
VS(substr("abcdef", 6), false);
VS(substr("abcdef", 3, 0), "");

$input = "Alien";
VS(str_pad($input, 10), "Alien     ");
VS(str_pad($input, 10, "-=", STR_PAD_LEFT), "-=-=-Alien");
VS(str_pad($input, 10, "_", STR_PAD_BOTH), "__Alien___");
VS(str_pad($input, 6 , "___"), "Alien_");
VS(str_pad($input, 6 , "\0"), "Alien\0");

VS(str_repeat("-=", 10), "-=-=-=-=-=-=-=-=-=-=");

{
  $text = "The quick brown fox jumped over the lazy dog.";
  VS(wordwrap($text, 20, "<br />\n"),
     "The quick brown fox<br />\njumped over the lazy<br />\ndog.");
}
{
  $text = "A very long woooooooooooord.";
  VS(wordwrap($text, 8, "\n", true), "A very\nlong\nwooooooo\nooooord.");
}

$orig = "I'll \"walk\" the <b>dog</b> now";
$a = htmlentities($orig);
VS($a, "I'll &quot;walk&quot; the &lt;b&gt;dog&lt;/b&gt; now");
VS(html_entity_decode($a), $orig);

VS(bin2hex(html_entity_decode("&nbsp;", 3)), "a0");
VS(bin2hex(html_entity_decode("&nbsp;", 3, "")), "c2a0");
VS(bin2hex(html_entity_decode("&nbsp;", 3, "UTF-8")), "c2a0");

VS(html_entity_decode("&amp; & &amp;", ENT_QUOTES, "UTF-8"), "& & &");
VS(html_entity_decode("&#00000000000000097; test &amp;", ENT_QUOTES, "UTF-8"), "a test &");

VS(bin2hex(html_entity_decode("&Egrave;")), "c8");
VS(bin2hex(html_entity_decode("&Egrave;", 3, "UTF-8")), "c388");

VS(html_entity_decode("&Alpha;"), "&Alpha;");
VS(bin2hex(html_entity_decode("&Alpha;", 3, "UTF-8")), "ce91");

$str = "A 'quote' is <b>bold</b>";
VS(htmlentities($str), "A 'quote' is &lt;b&gt;bold&lt;/b&gt;");
VS(htmlentities($str, ENT_QUOTES),
   "A &#039;quote&#039; is &lt;b&gt;bold&lt;/b&gt;");

VS(htmlentities("\xA0", ENT_COMPAT), "");
VS(htmlentities("\xc2\xA0", ENT_COMPAT, ""), "&nbsp;");
VS(htmlentities("\xc2\xA0", ENT_COMPAT, "UTF-8"), "&nbsp;");

VS(quoted_printable_encode("egfe \015\t"), "egfe=20=0D=09");

VS(quoted_printable_decode("=65=67=66=65="), "egfe");

VS(convert_uudecode("+22!L;W9E(%!(4\"$`\n`"), "I love PHP!");

VS(convert_uuencode("test\ntext text\r\n"),
   "0=&5S=`IT97AT('1E>'0-\"@``\n`\n");

VS(str_rot13("PHP 4.3.0"), "CUC 4.3.0");

VS(crc32("The quick brown fox jumped over the lazy dog."), 2191738434);

VERIFY(strlen(crypt("mypassword")));

VS(md5("apple"), "1f3870be274f6c49b3e31a0c6728957f");

VS(sha1("apple"), "d0be2dc421be4fcd0172e5afceea3970e2f3d940");

$trans = array("hello" => "hi", "hi" => "hello");
VS(strtr("hi all, I said hello", $trans), "hello all, I said hi");

VS(convert_cyr_string("abc", "a", "d"), "abc"); // sanity

VS(hebrev("test"), "test"); // sanity

VS(hebrevc("test"), "test"); // sanity

VS(nl_langinfo(AM_STR), "AM");

VS(sprintf("A%sB%dC", "test", 10), "AtestB10C");
VS(sprintf("%010s", "1101"), "0000001101");
VS(sprintf("%02d", "09"), "09");

VS(sprintf("(%s-%s)", "foo\0bar", "bar\0foo"), "(foo\0bar-bar\0foo)");
VS(sprintf("[%s]", "a\0b"), "[a\0b]");

VS(vsprintf("A%sB%dC", array("test", 10)), "AtestB10C");

VS(sscanf("SN/2350001", "SN/%d"), array(2350001));

VS(sscanf("SN/2350001", "SN/%d", $out), 1);
VS($out, 2350001);

VS(chr(92), "\\");

VS(ord("\\"), 92);

VS(money_format("%i", 1234.56), "1234.56");

VS(number_format(1234.56), "1,235");

VERIFY(strcmp("a", "b") < 0);
VERIFY(strcmp("a", "A") > 0);

VS(strncmp("a", "ab", 1), 0);

VERIFY(strnatcmp("a", "b") < 0);

VERIFY(strcasecmp("a", "a") == 0);
VERIFY(strcasecmp("a", "A") == 0);
VERIFY(strcasecmp("A", "a") == 0);
VERIFY(strcasecmp("A", "A") == 0);

VERIFY(strcasecmp("a", "b") < 0);
VERIFY(strcasecmp("a", "B") < 0);
VERIFY(strcasecmp("A", "b") < 0);
VERIFY(strcasecmp("A", "B") < 0);
VERIFY(strcasecmp("b", "a") > 0);
VERIFY(strcasecmp("B", "a") > 0);
VERIFY(strcasecmp("b", "A") > 0);
VERIFY(strcasecmp("B", "A") > 0);

VERIFY(strcasecmp("_", "a") < 0);
VERIFY(strcasecmp("_", "A") < 0);
VERIFY(strcasecmp("a", "_") > 0);
VERIFY(strcasecmp("A", "_") > 0);

VERIFY(strcasecmp("@", "`") < 0);
VERIFY(strcasecmp("`", "@") > 0);

VERIFY(strcasecmp("a", "a0") < 0);
VERIFY(strcasecmp("a", "A0") < 0);
VERIFY(strcasecmp("A", "a0") < 0);
VERIFY(strcasecmp("A", "A0") < 0);
VERIFY(strcasecmp("a0", "a") > 0);
VERIFY(strcasecmp("a0", "A") > 0);
VERIFY(strcasecmp("A0", "a") > 0);
VERIFY(strcasecmp("A0", "A") > 0);

VS(strncasecmp("a", "Ab", 1), 0);

VERIFY(strnatcasecmp("a", "Ab") < 0);

VERIFY(strcoll("a", "b") < 0);
VERIFY(strcoll("a", "A") > 0);

VS(substr_compare("abcde", "bc", 1, 2), 0);
VS(substr_compare("abcde", "de", -2, 2), 0);
VS(substr_compare("abcde", "bcg", 1, 2), 0);
VS(substr_compare("abcde", "BC", 1, 2, true), 0);
VS(substr_compare("abcde", "bc", 1, 3), 1);
VS(substr_compare("abcde", "cd", 1, 2), -1);

$email  = "name@example.com";
VS(strchr($email, "@"), "@example.com");

$text = "Line 1\nLine 2\nLine 3";
VS(strrchr($text, 10), "\nLine 3");

$email  = "name@example.com";
VS(strstr($email, "@"), "@example.com");
VS(strstr($email, "@", true), "name");
VS(strstr($email, "@", false), "@example.com");

VS(stristr("Hello World!", "earth"), false);
VS(stristr("APPLE", 97), "APPLE");

$text = "This is a Simple text.";
VS(strpbrk($text, "mi"), "is is a Simple text.");
VS(strpbrk($text, "S"), "Simple text.");

VS(strpos("abcdef abcdef", "a"), 0);
VS(strpos("abcdef abcdef", "a", 1), 7);
VS(strpos("abcdef abcdef", "A", 1), false);
VS(strpos("abcdef abcdef", "", 0), false);

VS(stripos("abcdef abcdef", "A", 1), 7);

VS(strrpos("abcdef abcdef", "a"), 7);
VS(strrpos("0123456789a123456789b123456789c", "7", -5), 17);
VS(strrpos("0123456789a123456789b123456789c", "7", 20), 27);
VS(strrpos("0123456789a123456789b123456789c", "7", 28), false);


VS(strripos("abcdef abcdef", "A"), 7);

$text = "This is a test";
VS(substr_count($text, "is"), 2);
VS(substr_count($text, "is", 3), 1);
VS(substr_count($text, "is", 3, 3), 0);
VS(substr_count("gcdgcdgcd", "gcdgcd"), 1);

VS(strspn("foo", "o", 1, 2), 2);

VS(strcspn("foo", "o", 1, 2), 0);

VS(strlen("test"), 4);

$ret = count_chars("Two Ts and one F.");
VS($ret[ord("T")], 2);

VS(str_word_count("Two Ts and one F."), 5);
VS(str_word_count("", 2), array());
VS(str_word_count(1, 2), array());
VS(str_word_count("1 2", 2), array());

VS(levenshtein("carrrot", "carrot"), 1);

VS(similar_text("carrrot", "carrot"), 6);

VS(soundex("carrot"), "C630");

VS(metaphone("carrot"), "KRT");

parse_str("first=value&arr[]=foo+bar&arr[]=baz", $output);
VS($output['first'], 'value');
VS($output['arr'][0], 'foo bar');
VS($output['arr'][1], 'baz');

parse_str('a[2][i]=3&a[4][i]=5', $output);
VS($output['a'][2]['i'], "3");
VS($output['a'][4]['i'], "5");

VS(wordwrap('foobar*foobar', 6, '*', true), 'foobar*foobar');
VS(wordwrap("12345 12345 12345 12345"), "12345 12345 12345 12345");
VS("12345 12345\n1234567890\n1234567890", wordwrap("12345 12345 1234567890 1234567890",12));
VS("12345 12345\n1234567890\n1234567890", wordwrap("12345 12345 1234567890 1234567890",12));
VS("12345\n12345\n12345\n12345", wordwrap("12345 12345 12345 12345",0));
VS("12345ab12345ab12345ab12345", wordwrap("12345 12345 12345 12345",0,"ab"));
VS("12345 12345ab1234567890ab1234567890", wordwrap("12345 12345 1234567890 1234567890",12,"ab"));
VS("123ab123ab123", wordwrap("123ab123ab123", 3, "ab"));
VS("123ab123ab123", wordwrap("123ab123ab123", 5, "ab"));
VS("123ab 123ab123", wordwrap("123  123ab123", 3, "ab"));
VS("123ab123ab123", wordwrap("123 123ab123", 5, "ab"));
VS("123 123ab123", wordwrap("123 123 123", 10, "ab"));
VS("123ab123ab123", wordwrap("123ab123ab123", 3, "ab", 1));
VS("123ab123ab123", wordwrap("123ab123ab123", 5, "ab", 1));
VS("123ab 12ab3ab123", wordwrap("123  123ab123", 3, "ab", 1));
VS("123 ab123ab123", wordwrap("123  123ab123", 5, "ab", 1));
VS("123  123ab 123", wordwrap("123  123  123", 8, "ab", 1));
VS("123 ab12345 ab123", wordwrap("123  12345  123", 8, "ab", 1));
VS("1ab2ab3ab4", wordwrap("1234", 1, "ab", 1));
VS("12345|12345|67890", wordwrap("12345 1234567890", 5, "|", 1));
VS("123|==1234567890|==123", wordwrap("123 1234567890 123", 10, "|==", 1));

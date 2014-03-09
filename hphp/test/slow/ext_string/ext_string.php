<?php

//////////////////////////////////////////////////////////////////////

var_dump(addcslashes("ABCDEFGH\n", "A..D\n"));

var_dump(stripcslashes("\\A\\B\\C\\DEFGH\\n"));

var_dump(addslashes("'\"\\\n"));

var_dump(stripslashes("\\'\\\"\\\\\n"));

var_dump(bin2hex("ABC\n"));

var_dump(hex2bin("4142430a"));

var_dump(nl2br("A\nB"));

var_dump(quotemeta(". \\ + * ? [ ^ ] ( $ )"));

var_dump(strlen(str_shuffle("ABC")));

var_dump(strrev("ABC"));

var_dump(strtolower("ABC"));

var_dump(strtoupper("abc"));

var_dump(ucfirst("abc"));

var_dump(lcfirst("ABC"));

var_dump(ucwords("abc def"));

$text = "<p>Test paragraph.</p><!-- Comment --> ".
  "<a href=\"#fragment\">Other text</a>";
var_dump(strip_tags($text));
var_dump(strip_tags($text, "<p><a>"));

var_dump(trim(" abc "));

var_dump(ltrim(" abc "));

var_dump(rtrim(" abc "));

var_dump(chop(" abc "));

{
  $metric = "create stuff";
  $pieces = explode(" ", $metric, 1);
  var_dump(count($pieces));
  var_dump($pieces[0]);
}
{
  $metric = "create stuff";
  $pieces = explode(" ", $metric, 0);
  var_dump(count($pieces));
  var_dump($pieces[0]);
}
{
  $pizza = "piece1 piece2 piece3 piece4 piece5 piece6";
  $pieces = explode(" ", $pizza);
  var_dump($pieces[0]);
  var_dump($pieces[1]);
}
{
  $str = "one|two|three|four";
  $ret = explode("|", $str, 2);
  var_dump(count($ret));
  var_dump($ret[0]);
  var_dump($ret[1]);
}
{
  $str = "one|two|three|four";
  $ret = explode("|", $str, -1);
  var_dump(count($ret));
  var_dump($ret[0]);
  var_dump($ret[1]);
  var_dump($ret[2]);
}
{
  $str = "ab";
  $ret = explode("b", $str);
  var_dump($ret[0]);
  var_dump($ret[1]);
}
{
  $str = "b";
  $ret = explode("b", $str);
  var_dump($ret[0]);
  var_dump($ret[1]);
}
{
  $str = "bb";
  $ret = explode("b", $str);
  var_dump($ret[0]);
  var_dump($ret[1]);
  var_dump($ret[2]);
}
{
  $str = "";
  $ret = explode("b", $str);
  var_dump($ret[0]);
}

{
  $arr = array("lastname", "email", "phone");
  var_dump(implode(",", $arr));
}
{
  $arr = array("lastname", "", "phone");
  var_dump(implode(",", $arr));
}
{
  $arr = array("", "email", "phone");
  var_dump(implode(",", $arr));
}
{
  $arr = array("", "", "");
  var_dump(implode(",", $arr));
}
{
  $arr = array();
  var_dump(implode(",", $arr));
}

$arr = array("lastname", "email", "phone");
var_dump(join(",", $arr));

$str = "Hello Friend";
$arr1 = str_split($str);
var_dump(count($arr1));
var_dump($arr1[0]);
var_dump($arr1[1]);

$arr2 = str_split($str, 3);
var_dump(count($arr2) == 4);
var_dump($arr2[0]);
var_dump($arr2[1]);

// TODO(#2512685): this behaves differently in the interpreter vs. jit
// $ret = chunk_split("ABCD", 2);
// var_dump($ret, "AB\r\nCD\r\n");

$s = "This is\tan ";
$tok = strtok($s, " \n\t");
while ($tok) {
  $tokens[] = $tok;
  $tok = strtok(" \n\t");
}
var_dump($tokens);

{
  var_dump(str_replace("%body%", "black", "<body text='%body%'>"));
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
  var_dump(str_replace($vowels, "", "Hello World of PHP"));
}
{
  $phrase  = "You should eat fruits, vegetables, and fiber every day.";
  $healthy = array("fruits", "vegetables", "fiber");
  $yummy   = array("pizza", "beer", "ice cream");
  var_dump(str_replace($healthy, $yummy, $phrase));
}
{
  $str = str_replace("ll", "", "good golly miss molly!",
                              $count);
  var_dump($count);
}
{
  $letters = array("a", "p");
  $fruit = array("apple", "pear");
  $text = "a p";
  var_dump(str_replace($letters, $fruit, $text));
}


var_dump(str_ireplace("%body%", "black", "<body text='%BODY%'>"));
var_dump(str_ireplace("%body%", "Black", "<body Text='%BODY%'>"));

$var = "ABCDEFGH:/MNRPQR/";
var_dump(substr_replace($var, "bob", 0));
var_dump(substr_replace($var, "bob", 0, strlen($var)));
var_dump(substr_replace($var, "bob", 0, 0));

var_dump(substr_replace($var, "bob", 10, -1));
var_dump(substr_replace($var, "bob", -7, -1));

var_dump(substr_replace($var, "", 10, -1));

var_dump(substr("abcdef", 1));
var_dump(substr("abcdef", 1, 3));
var_dump(substr("abcdef", 0, 4));
var_dump(substr("abcdef", 0, 8));
var_dump(substr("abcdef", -1, 1));
var_dump(substr("abcdef", 6));
var_dump(substr("abcdef", 3, 0));

$input = "Alien";
var_dump(str_pad($input, 10));
var_dump(str_pad($input, 10, "-=", STR_PAD_LEFT));
var_dump(str_pad($input, 10, "_", STR_PAD_BOTH));
var_dump(str_pad($input, 6 , "___"));
var_dump(str_pad($input, 6 , "\0"));

var_dump(str_repeat("-=", 10));

{
  $text = "The quick brown fox jumped over the lazy dog.";
  var_dump(wordwrap($text, 20, "<br />\n"));
}
{
  $text = "A very long woooooooooooord.";
  var_dump(wordwrap($text, 8, "\n", true));
}

$orig = "I'll \"walk\" the <b>dog</b> now";
$a = htmlentities($orig);
var_dump($a);
var_dump(html_entity_decode($a));

var_dump(bin2hex(html_entity_decode("&nbsp;", 3)));
var_dump(bin2hex(html_entity_decode("&nbsp;", 3, "")));
var_dump(bin2hex(html_entity_decode("&nbsp;", 3, "UTF-8")));

var_dump(html_entity_decode("&amp; & &amp;", ENT_QUOTES, "UTF-8"));
var_dump(html_entity_decode("&#00000000000000097; test &amp;", ENT_QUOTES, "UTF-8"));

var_dump(bin2hex(html_entity_decode("&Egrave;")));
var_dump(bin2hex(html_entity_decode("&Egrave;", 3, "UTF-8")));

var_dump(html_entity_decode("&Alpha;"));
var_dump(bin2hex(html_entity_decode("&Alpha;", 3, "UTF-8")));

$str = "A 'quote' is <b>bold</b>";
var_dump(htmlentities($str));
var_dump(htmlentities($str, ENT_QUOTES));

var_dump(htmlentities("\xA0", ENT_COMPAT));
var_dump(htmlentities("\xc2\xA0", ENT_COMPAT, ""));
var_dump(htmlentities("\xc2\xA0", ENT_COMPAT, "UTF-8"));

var_dump(quoted_printable_encode("egfe \015\t"));

var_dump(quoted_printable_decode("=65=67=66=65="));

var_dump(convert_uudecode("+22!L;W9E(%!(4\"$`\n`"));

var_dump(convert_uuencode("test\ntext text\r\n"));

var_dump(str_rot13("PHP 4.3.0"));

var_dump(crc32("The quick brown fox jumped over the lazy dog."));

var_dump(strlen(crypt("mypassword")));

var_dump(md5("apple"));

var_dump(sha1("apple"));

$trans = array("hello" => "hi", "hi" => "hello");
var_dump(strtr("hi all, I said hello", $trans));

var_dump(convert_cyr_string("abc", "a", "d")); // sanity

var_dump(hebrev("test")); // sanity

var_dump(hebrevc("test")); // sanity

var_dump(nl_langinfo(AM_STR));

var_dump(sprintf("A%sB%dC", "test", 10));
var_dump(sprintf("%010s", "1101"));
var_dump(sprintf("%02d", "09"));

var_dump(sprintf("(%s-%s)", "foo\0bar", "bar\0foo"));
var_dump(sprintf("[%s]", "a\0b"));

var_dump(vsprintf("A%sB%dC", array("test", 10)));

var_dump(sscanf("SN/2350001", "SN/%d"));

var_dump(sscanf("SN/2350001", "SN/%d", $out));
var_dump($out);

var_dump(chr(92));

var_dump(ord("\\"));

var_dump(money_format("%i", 1234.56));

var_dump(number_format(1234.56));

var_dump(strcmp("a", "b"));
var_dump(strcmp("a", "A"));

var_dump(strncmp("a", "ab", 1));

var_dump(strnatcmp("a", "b"));

var_dump(strcasecmp("a", "a"));
var_dump(strcasecmp("a", "A"));
var_dump(strcasecmp("A", "a"));
var_dump(strcasecmp("A", "A"));

var_dump(strcasecmp("a", "b"));
var_dump(strcasecmp("a", "B"));
var_dump(strcasecmp("A", "b"));
var_dump(strcasecmp("A", "B"));
var_dump(strcasecmp("b", "a"));
var_dump(strcasecmp("B", "a"));
var_dump(strcasecmp("b", "A"));
var_dump(strcasecmp("B", "A"));

var_dump(strcasecmp("_", "a"));
var_dump(strcasecmp("_", "A"));
var_dump(strcasecmp("a", "_"));
var_dump(strcasecmp("A", "_"));

var_dump(strcasecmp("@", "`"));
var_dump(strcasecmp("`", "@"));

var_dump(strcasecmp("a", "a0"));
var_dump(strcasecmp("a", "A0"));
var_dump(strcasecmp("A", "a0"));
var_dump(strcasecmp("A", "A0"));
var_dump(strcasecmp("a0", "a"));
var_dump(strcasecmp("a0", "A"));
var_dump(strcasecmp("A0", "a"));
var_dump(strcasecmp("A0", "A"));

var_dump(strncasecmp("a", "Ab", 1));

var_dump(strnatcasecmp("a", "Ab"));

var_dump(strcoll("a", "b"));
var_dump(strcoll("a", "A"));

var_dump(substr_compare("abcde", "bc", 1, 2));
var_dump(substr_compare("abcde", "de", -2, 2));
var_dump(substr_compare("abcde", "bcg", 1, 2));
var_dump(substr_compare("abcde", "BC", 1, 2, true));
var_dump(substr_compare("abcde", "bc", 1, 3));
var_dump(substr_compare("abcde", "cd", 1, 2));

$email  = "name@example.com";
var_dump(strchr($email, "@"));

$text = "Line 1\nLine 2\nLine 3";
var_dump(strrchr($text, 10));

$email  = "name@example.com";
var_dump(strstr($email, "@"));
var_dump(strstr($email, "@", true));
var_dump(strstr($email, "@", false));

var_dump(stristr("Hello World!", "earth"));
var_dump(stristr("APPLE", 97));

$text = "This is a Simple text.";
var_dump(strpbrk($text, "mi"));
var_dump(strpbrk($text, "S"));

var_dump(strpos("abcdef abcdef", "a"));
var_dump(strpos("abcdef abcdef", "a", 1));
var_dump(strpos("abcdef abcdef", "A", 1));
var_dump(strpos("abcdef abcdef", "", 0));

var_dump(stripos("abcdef abcdef", "A", 1));

var_dump(strrpos("abcdef abcdef", "a"));
var_dump(strrpos("0123456789a123456789b123456789c", "7", -5));
var_dump(strrpos("0123456789a123456789b123456789c", "7", 20));
var_dump(strrpos("0123456789a123456789b123456789c", "7", 28));


var_dump(strripos("abcdef abcdef", "A"));

$text = "This is a test";
var_dump(substr_count($text, "is"));
var_dump(substr_count($text, "is", 3));
var_dump(substr_count($text, "is", 3, 3));
var_dump(substr_count("gcdgcdgcd", "gcdgcd"));

var_dump(strspn("foo", "o", 1, 2));

var_dump(strcspn("foo", "o", 1, 2));

var_dump(strlen("test"));

$ret = count_chars("Two Ts and one F.");
var_dump($ret[ord("T")]);

var_dump(str_word_count("Two Ts and one F."));
var_dump(str_word_count("", 2));
var_dump(str_word_count("1", 2));
var_dump(str_word_count("1 2", 2));

var_dump(levenshtein("carrrot", "carrot"));

var_dump(similar_text("carrrot", "carrot"));

var_dump(soundex("carrot"));

var_dump(metaphone("carrot"));

parse_str("first=value&arr[]=foo+bar&arr[]=baz", $output);
var_dump($output['first']);
var_dump($output['arr'][0]);
var_dump($output['arr'][1]);

parse_str('a[2][i]=3&a[4][i]=5', $output);
var_dump($output['a'][2]['i']);
var_dump($output['a'][4]['i']);

var_dump(wordwrap('foobar*foobar', 6, '*', true));
var_dump(wordwrap("12345 12345 12345 12345"));
var_dump(wordwrap("12345 12345 1234567890 1234567890",12));
var_dump(wordwrap("12345 12345 1234567890 1234567890",12));
var_dump(wordwrap("12345 12345 12345 12345",0));
var_dump(wordwrap("12345 12345 12345 12345",0,"ab"));
var_dump(wordwrap("12345 12345 1234567890 1234567890",12,"ab"));
var_dump(wordwrap("123ab123ab123", 3, "ab"));
var_dump(wordwrap("123ab123ab123", 5, "ab"));
var_dump(wordwrap("123  123ab123", 3, "ab"));
var_dump(wordwrap("123 123ab123", 5, "ab"));
var_dump(wordwrap("123 123 123", 10, "ab"));
var_dump(wordwrap("123ab123ab123", 3, "ab", 1));
var_dump(wordwrap("123ab123ab123", 5, "ab", 1));
var_dump(wordwrap("123  123ab123", 3, "ab", 1));
var_dump(wordwrap("123  123ab123", 5, "ab", 1));
var_dump(wordwrap("123  123  123", 8, "ab", 1));
var_dump(wordwrap("123  12345  123", 8, "ab", 1));
var_dump(wordwrap("1234", 1, "ab", 1));
var_dump(wordwrap("12345 1234567890", 5, "|", 1));
var_dump(wordwrap("123 1234567890 123", 10, "|==", 1));

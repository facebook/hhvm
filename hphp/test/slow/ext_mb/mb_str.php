<?php


mb_parse_str("first=value&arr[]=foo+bar&arr[]=baz", $output);
var_dump($output['first']);
var_dump($output['arr[]']); // bug in mb_parse_str not following PHP's

$date = "04/30/1973";
$ret = mb_split("[/.-]", $date);
var_dump($ret[0]);
var_dump($ret[1]);
var_dump($ret[2]);

var_dump(mb_strcut("abcdef", 1));
var_dump(mb_strcut("abcdef", 1, 3));
var_dump(mb_strcut("abcdef", 0, 4));
var_dump(mb_strcut("abcdef", 0, 8));
var_dump(mb_strcut("abcdef", -1, 1));

var_dump(mb_strcut("\xC3\x9C"."bcdef", 2));
var_dump(mb_strcut("\xC3\x9C"."bcdef", 2, 3));
var_dump(mb_strcut("\xC3\x9C"."bcdef", 0, 4) === "\xC3\x9C"."bc");
var_dump(mb_strcut("\xC3\x9C"."bcdef", 0, 8) === "\xC3\x9C"."bcdef");
var_dump(mb_strcut("\xC3\x9C"."bcdef", -1, 1));

var_dump(mb_strimwidth("long string", 0, 6, "..>"));
var_dump(mb_strimwidth("\xC3\x9C"."long string", 0, 6, "..>"));

var_dump(mb_stripos("abcdef abcdef", "A", 1));
var_dump(mb_stripos("abcdef\xC3\x9C"."abcdef", "A", 1));

var_dump(mb_stristr("Hello World!", "earth"));

var_dump(mb_strlen("test"));
var_dump(mb_strlen("Pr\xC3\x9C\xC3\x9D"."fung"));

var_dump(mb_strpos("abcdef abcdef", "a", 1));
var_dump(mb_strpos("abcdef\xC3\x9C"."abcdef", "a", 1));
var_dump(mb_strpos("abcdef\xC3\x9C"."abcdef", "A", 1));

$text = "Line 1\nLine 2\nLine 3";
var_dump(mb_strrchr($text, "\n"));

$text = "Line 1\nLine 2\xC3\x9C"."Line 3";
var_dump(strrchr($text, "\x9C") === "\x9C"."Line 3");
// mb_strrchr behaves differently in different versions of
// libmbfl (https://github.com/facebook/hiphop-php/issues/68)
var_dump(mb_strrchr($text, "\x9C") === false ||
         mb_strrchr($text, "\x9C") === "Line 3");

$text = "Line 1\nLine 2\nLine 3";
var_dump(mb_strrichr($text, "l"));

var_dump(mb_strripos("abcdef abcdef", "A"));
var_dump(mb_strripos("abcdef\xC3\x9C"."abcdef", "A"));


var_dump(mb_strrpos("abcdef abcdef", "a"));
var_dump(mb_strrpos("abcdef\xC3\x9C"."abcdef", "a"));


$email = "name@example.com";
var_dump(mb_strstr($email, "@"));

$str = "Mary Had A Little Lamb and She LOVED It So";
$str = mb_strtolower($str);
var_dump($str);
var_dump(mb_strtolower("ABC"));

$str = "Mary Had A Little Lamb and She LOVED It So";
$str = mb_strtoupper($str);
var_dump($str);
var_dump(mb_strtoupper("abc"));

var_dump(mb_strwidth("Pr\xC3\x9C"."fung"));

/* Set with Unicode U+3013 (GETA MARK) */
mb_substitute_character(0x3013);
var_dump(mb_substitute_character() === 0x3013);

/* Set hex format */
mb_substitute_character("long");

/* Display current setting */
var_dump(mb_substitute_character());


var_dump(mb_substr_count("This is a test", "is"));
$text = "This is a test";
var_dump(mb_substr_count($text, "is"));

// different from substr_count
// mb_strrchr behaves differently in different versions of
// libmbfl (https://github.com/facebook/hiphop-php/issues/68)
var_dump(mb_substr_count("gcdgcdgcd", "gcdgcd") === 2 ||
         mb_substr_count("gcdgcdgcd", "gcdgcd") === 1);

var_dump(mb_substr("abcdef", 1));
var_dump(mb_substr("abcdef", 1, 3));
var_dump(mb_substr("abcdef", 0, 4));
var_dump(mb_substr("abcdef", 0, 8));
var_dump(mb_substr("abcdef", -1, 1));

var_dump(mb_substr("\xC3\x9C"."bcdef", 1));
var_dump(mb_substr("\xC3\x9C"."bcdef", 1, 3));
var_dump(mb_substr("\xC3\x9C"."bcdef", 0, 4) === "\xC3\x9C"."bcd");
var_dump(mb_substr("\xC3\x9C"."bcdef", 0, 8) === "\xC3\x9C"."bcdef");
var_dump(mb_substr("\xC3\x9C"."bcdef", -1, 1));

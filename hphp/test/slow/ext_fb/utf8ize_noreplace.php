<?php

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
$INVALID_UTF_8_STRING = "\xe2\x82\x28";

$s = "hon\xE7k";
var_dump(fb_utf8ize($s));
var_dump($s);

$s = "test\xE0\xB0\xB1\xE0";
var_dump(fb_utf8ize($s));
var_dump($s);

$s = "test\xE0\xB0\xB1\xE0\xE0";
var_dump(fb_utf8ize($s));
var_dump($s);

$s = "\xfc";
var_dump(fb_utf8ize($s));
var_dump($s);

$s = "\xfc\xfc";
var_dump(fb_utf8ize($s));
var_dump($s);

// We intentionally consider null bytes invalid sequences.
$s = "abc\x00def";
var_dump(fb_utf8ize($s));
var_dump($s);

// ICU treats this as as two code points.
// The old implementation treated this as three code points.
$s = $INVALID_UTF_8_STRING;
var_dump(fb_utf8ize($s));
var_dump($s);

//// utf8_strlen
echo "====\n";

var_dump(fb_utf8_strlen(""));
var_dump(fb_utf8_strlen("a"));
var_dump(fb_utf8_strlen("ab"));
  // Valid UTF-8 sequence returns code point count.
var_dump(fb_utf8_strlen("\ub098\ub294"));
var_dump(fb_utf8_strlen($INVALID_UTF_8_STRING));

$s = "abc\x00def";
var_dump(strlen($s));
var_dump(fb_utf8_strlen($s));
fb_utf8ize($s);
var_dump(strlen($s));
var_dump(fb_utf8_strlen($s));

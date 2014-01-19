<?php

// See utf8ize.php
$INVALID_UTF_8_STRING = "\xe2\x82\x28";

// Falsey inputs
var_dump(fb_utf8_substr("", 0, 0));
var_dump(fb_utf8_substr("", 0, 1));
var_dump(fb_utf8_substr("hon\xE7k", 0, INT_MAX));
var_dump(fb_utf8_substr("hon\xE7k", 0, 3));
var_dump(fb_utf8_substr("hon\xE7k", -4, INT_MAX));

// Common cases
var_dump(fb_utf8_substr("X", 0, 1));
var_dump(fb_utf8_substr("Hello", 0, INT_MAX));
var_dump(fb_utf8_substr("Hello", 1, 2));
var_dump(fb_utf8_substr("Pr\u00DC\u00DDx", 2, 2));

// Negative start
var_dump(fb_utf8_substr("abcdef", -1, INT_MAX));
var_dump(fb_utf8_substr("abcdef", -2, INT_MAX));
var_dump(fb_utf8_substr("abcdef", -3, 1));
var_dump(fb_utf8_substr("", -1, 1));
var_dump(fb_utf8_substr("X", -1, 1));
var_dump(fb_utf8_substr("XY", -1, 1));
var_dump(fb_utf8_substr("Pr\u00DC\u00DDx", -3, 2));

// Negative lengths
var_dump(fb_utf8_substr("abcdef", 0, -1));
var_dump(fb_utf8_substr("abcdef", 2, -1));
var_dump(fb_utf8_substr("abcdef", 4, -4));
var_dump(fb_utf8_substr("abcdef", -3, -1));

// Invalid sequence
var_dump(fb_utf8_substr($INVALID_UTF_8_STRING, 0), "\uFFFD\x28");

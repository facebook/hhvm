<?php

// See utf8ize.php
$INVALID_UTF_8_STRING = "\xe2\x82\x28";

// Falsey inputs
echo json_encode(fb_utf8_substr("", 0, 0)), PHP_EOL;
echo json_encode(fb_utf8_substr("", 0, 1)), PHP_EOL;
echo json_encode(fb_utf8_substr("hon\xE7k", 0, PHP_INT_MAX)), PHP_EOL;
echo json_encode(fb_utf8_substr("hon\xE7k", 0, 3)), PHP_EOL;
echo json_encode(fb_utf8_substr("hon\xE7k", -4, PHP_INT_MAX)), PHP_EOL;

// Common cases
echo json_encode(fb_utf8_substr("X", 0, 1)), PHP_EOL;
echo json_encode(fb_utf8_substr("Hello", 0, PHP_INT_MAX)), PHP_EOL;
echo json_encode(fb_utf8_substr("Hello", 1, 2)), PHP_EOL;
echo json_encode(fb_utf8_substr("Pr\u00DC\u00DDx", 2, 2)), PHP_EOL;

// Negative start
echo json_encode(fb_utf8_substr("abcdef", -1, PHP_INT_MAX)), PHP_EOL;
echo json_encode(fb_utf8_substr("abcdef", -2, PHP_INT_MAX)), PHP_EOL;
echo json_encode(fb_utf8_substr("abcdef", -3, 1)), PHP_EOL;
echo json_encode(fb_utf8_substr("", -1, 1)), PHP_EOL;
echo json_encode(fb_utf8_substr("X", -1, 1)), PHP_EOL;
echo json_encode(fb_utf8_substr("XY", -1, 1)), PHP_EOL;
echo json_encode(fb_utf8_substr("Pr\u00DC\u00DDx", -3, 2)), PHP_EOL;

// Negative lengths
echo json_encode(fb_utf8_substr("abcdef", 0, -1)), PHP_EOL;
echo json_encode(fb_utf8_substr("abcdef", 2, -1)), PHP_EOL;
echo json_encode(fb_utf8_substr("abcdef", 4, -4)), PHP_EOL;
echo json_encode(fb_utf8_substr("abcdef", -3, -1)), PHP_EOL;

// Invalid sequence
echo json_encode(fb_utf8_substr($INVALID_UTF_8_STRING, 0)), PHP_EOL;
echo json_encode("\uFFFD\x28"), PHP_EOL;

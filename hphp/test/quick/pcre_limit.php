<?php

$a = 'baab' . str_repeat('a', 1000000);
$b = preg_replace('/b.*b/', '', $a);
var_dump(preg_last_error());

ini_set('pcre.backtrack_limit', PHP_INT_MAX);

$b = preg_replace('/b.*b/', '', $a);
var_dump(preg_last_error());

/* Test that pcre works for bad unicode that is correct utf-8 */
$bad_unicode = "\xef\xbf\xbf"; // \uFFFF
// regular expression from xml_sanitize
$re = '/[^\x{0009}\x{000a}\x{000d}\x{0020}-\x{D7FF}\x{E000}-\x{FFFD}' .
    '\x{10000}-\x{10FFFF}]+/u';
var_dump(preg_replace($re, "\xef\xbf\xbd", $bad_unicode) == "\xef\xbf\xbd");

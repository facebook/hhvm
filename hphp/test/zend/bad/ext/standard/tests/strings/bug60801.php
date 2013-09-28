<?php
$haystack = "foob\x00ar";
$needle = "a\x00b";

var_dump(strpbrk($haystack, 'ar'));
var_dump(strpbrk($haystack, "\x00"));
var_dump(strpbrk($haystack, $needle));
var_dump(strpbrk('foobar', $needle));
var_dump(strpbrk("\x00", $needle));
var_dump(strpbrk('xyz', $needle));
var_dump(strpbrk($haystack, 'xyz'));
?>
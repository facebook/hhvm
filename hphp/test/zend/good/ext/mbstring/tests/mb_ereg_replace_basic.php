<?php
/* Prototype  : string mb_ereg_replace(string $pattern, string $replacement, 
 * string $string [, string o$ption])
 * Description: Replace regular expression for multibyte string 
 * Source code: ext/mbstring/php_mbregex.c
 */

/*
 * Test Basic Functionality of mb_ereg_replace()
 */

echo "*** Testing mb_ereg_replace() : basic functionality ***\n";

mb_internal_encoding('UTF-8');
mb_regex_encoding('UTF-8');

$string_ascii = b'abc def';
$string_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII=');

echo "\n-- ASCII string 1 --\n";
$result_1 = mb_ereg_replace(b'(.*)def', b'\\1 123', $string_ascii);
var_dump(bin2hex($result_1));

echo "\n-- ASCII string 2 --\n";
$result_2 = mb_ereg_replace(b'123', b'abc', $string_ascii);
var_dump(bin2hex($result_2));

echo "\n-- Multibyte string 1 --\n";
$regex1 = base64_decode('KOaXpeacrOiqnikuKj8oWzEtOV0rKQ==');   //Japanese regex in UTF-8
$result_3 = mb_ereg_replace($regex1, b'\\1_____\\2', $string_mb);
var_dump(bin2hex($result_3));

echo "\n-- Multibyte string 2 --\n";
$regex2 = base64_decode('5LiW55WM');
$result_4 = mb_ereg_replace($regex2, b'_____', $string_mb);
var_dump(bin2hex($result_4));

echo "Done";
?>

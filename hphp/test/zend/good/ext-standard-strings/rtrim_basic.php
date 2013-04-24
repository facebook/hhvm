<?php

/* Prototype  : string rtrim  ( string $str  [, string $charlist  ] )
 * Description: Strip whitespace (or other characters) from the end of a string.
 * Source code: ext/standard/string.c
*/

echo "*** Testing rtrim() : basic functionality ***\n";

$text  = "---These are a few words---  \t\r\n\0\x0B  ";
$hello  = "!===Hello World===!";
$alpha = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
$binary = "Example string\x0A\x0D";



echo "\n-- Trim string with all white space characters --\n";
var_dump(rtrim($text));

echo "\n-- Trim non-whitespace from a string --\n"; 
var_dump(rtrim($hello, "=!"));

echo "\n-- Trim some non-white space characters from a string --\n"; 
var_dump(rtrim($hello, "!dlWro="));

echo "\n-- Trim some non-white space characters from a string using a character range --\n"; 
var_dump(rtrim($alpha, "A..Z"));

echo "\n-- Trim the ASCII control characters at the beginning of a string --\n";
var_dump(rtrim($binary, "\x00..\x1F"));

?>
===DONE===
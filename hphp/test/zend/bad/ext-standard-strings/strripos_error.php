<?php
/* Prototype  : int strripos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of last occurrence of a case-insensitive 'needle' in a 'haystack'
 * Source code: ext/standard/string.c
*/

echo "*** Testing strripos() function: error conditions ***";
echo "\n-- With Zero arguments --";
var_dump( strripos() );

echo "\n-- With less than expected number of arguments --";
var_dump( strripos("String") );

echo "\n-- With more than expected number of arguments --";
var_dump( strripos("string", "String", 1, 'extra_arg') );
?>
===DONE===
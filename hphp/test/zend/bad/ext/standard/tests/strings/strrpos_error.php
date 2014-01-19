<?php
/* Prototype  : int strrpos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of last occurrence of 'needle' in 'haystack'.
 * Source code: ext/standard/string.c
*/

echo "*** Testing strrpos() function: error conditions ***";
echo "\n-- With Zero arguments --";
var_dump( strrpos() );

echo "\n-- With less than expected number of arguments --";
var_dump( strrpos("String") );

echo "\n-- With more than expected number of arguments --";
var_dump( strrpos("string", "String", 1, 'extra_arg') );
echo "*** Done ***";
?>
<?php
/* Prototype  : string nl2br(string $str)
 * Description: Inserts HTML line breaks before all newlines in a string.
 * Source code: ext/standard/string.c
*/

echo "*** Testing nl2br() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing nl2br() function with Zero arguments --";
var_dump( nl2br() );

//Test nl2br with one more than the expected number of arguments
echo "\n-- Testing nl2br() function with more than expected no. of arguments --";
$str = 'string_val';
$extra_arg = 10;
var_dump( nl2br($str, true, $extra_arg) );

echo "Done";
?>
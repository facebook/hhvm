<?php
/* Prototype  : int strnatcmp  ( string $str1  , string $str2  )
 * Description: String comparisons using a "natural order" algorithm
 * Source code: ext/standard/string.c
*/
echo "*** Testing strnatcmp() : error conditions ***\n";

echo "-- Testing strnatcmp() function with Zero arguments --\n";
var_dump( strnatcmp() );

echo "\n\n-- Testing strnatcmp() function with more than expected no. of arguments --\n";
$str1 = "abc1";
$str2 = "ABC1";
$extra_arg = 10;
var_dump( strnatcmp( $str1, $str2, $extra_arg) );

?>
===DONE===
<?php
/* Prototype  : string money_format  ( string $format  , float $number  )
 * Description: Formats a number as a currency string
 * Source code: ext/standard/string.c
*/

// ===========================================================================================
// = We do not test for exact return-values, as those might be different between OS-versions =
// ===========================================================================================

$string = '%14#8.2n';
$value = 1234.56;
$extra_arg = 10;

echo "*** Testing money_format() : error conditions ***\n";

echo "\n-- Testing money_format() function with no arguments --\n";
var_dump( money_format() );

echo "\n-- Testing money_format() function with insufficient arguments --\n";
var_dump( money_format($string) );

echo "\n-- Testing money_format() function with more than expected no. of arguments --\n";

var_dump( money_format($string, $value, $extra_arg) );

?>
===DONE===
<?php
/* Prototype  : string vsprintf(string $format , array $args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
 */

echo "*** Testing vsprintf() : error conditions ***\n";

// initialising the required variables
$format = "%s";
$args = array("hello");
$extra_arg = "extra arg";

// Zero arguments
echo "\n-- Testing vsprintf() function with Zero arguments --\n";
var_dump( vsprintf() );

echo "\n-- Testing vsprintf() function with less than expected no. of arguments --\n";
var_dump( vsprintf($format) );  

echo "\n-- testing vsprintf() function with more than expected no. of arguments --\n";
var_dump( vsprintf($format, $args, $extra_arg) );

echo "Done";
?>
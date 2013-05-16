<?php
/* Prototype  : int vprintf(string $format , array $args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
 */

echo "*** Testing vprintf() : error conditions ***\n";

// initialising the required variables
$format = "%s";
$args = array("hello");
$extra_arg = "extra arg";

// Zero arguments
echo "\n-- Testing vprintf() function with Zero arguments --\n";
var_dump( vprintf() );

echo "\n-- Testing vprintf() function with less than expected no. of arguments --\n";
var_dump( vprintf($format) );  

echo "\n-- testing vprintf() function with more than expected no. of arguments --\n";
var_dump( vprintf($format, $args, $extra_arg) );

?>
===DONE===
<?php
/* Prototype  : mixed var_export(mixed var [, bool return])
 * Description: Outputs or returns a string representation of a variable 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */

echo "*** Testing var_export() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing var_export() function with Zero arguments --\n";
var_dump( var_export() );

//Test var_export with one more than the expected number of arguments
echo "\n-- Testing var_export() function with more than expected no. of arguments --\n";
$var = 1;
$return = true;
$extra_arg = 10;
var_dump( var_export($var, $return, $extra_arg) );

?>
===DONE===

<?php
/* Prototype  : array localtime([int timestamp [, bool associative_array]])
 * Description: Returns the results of the C system call localtime as an associative array 
 * if the associative_array argument is set to 1 other wise it is a regular array 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing localtime() : usage variation ***\n";

date_default_timezone_set("UTC");
// Initialise function arguments not being substituted (if any)
$is_associative = true;

echo "\n-- Testing localtime() function with 'float 12.3456789000e10' to timestamp --\n";
$timestamp = 12.3456789000e10;
var_dump( localtime($timestamp) );
var_dump( localtime($timestamp, $is_associative) );

echo "\n-- Testing localtime() function with 'float -12.3456789000e10' to timestamp --\n";
$timestamp = -12.3456789000e10;
var_dump( localtime($timestamp) );
var_dump( localtime($timestamp, $is_associative) );

?>
===DONE===

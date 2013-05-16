<?php
/* Prototype  : array localtime([int timestamp [, bool associative_array]])
 * Description: Returns the results of the C system call localtime as an associative array 
 * if the associative_array argument is set to 1 other wise it is a regular array 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing localtime() : basic functionality ***\n";

date_default_timezone_set("UTC");

// Initialise all required variables
$timestamp = 10;
$associative_array = true;

// Calling localtime() with all possible arguments
var_dump( localtime($timestamp, $associative_array) );

// Calling localtime() with possible optional arguments
var_dump( localtime($timestamp) );

// Calling localtime() with mandatory arguments
var_dump( localtime() );

?>
===DONE===
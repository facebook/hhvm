<?php
/* Prototype  : int idate(string format [, int timestamp])
 * Description: Format a local time/date as integer 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing idate() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$format = 'Y';
date_default_timezone_set("Asia/Calcutta");

echo "\n-- Testing idate() function with float 12.3456789000e10 to timestamp --\n";
$timestamp = 12.3456789000e10;
var_dump( idate($format, $timestamp) );

echo "\n-- Testing idate() function with float -12.3456789000e10 to timestamp --\n";
$timestamp = -12.3456789000e10;
var_dump( idate($format, $timestamp) );
      
?>
===DONE===
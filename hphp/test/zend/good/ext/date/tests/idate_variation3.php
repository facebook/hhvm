<?hh
/* Prototype  : int idate(string format [, int timestamp])
 * Description: Format a local time/date as integer
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */

echo "*** Testing idate() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$format = 'Y';
date_default_timezone_set("Asia/Calcutta");

echo "\n-- Testing idate() function with int 123456789000 to timestamp --\n";
$timestamp = 123456789000;
var_dump( idate($format, $timestamp) );

echo "\n-- Testing idate() function with int -123456789000 to timestamp --\n";
$timestamp = -123456789000;
var_dump( idate($format, $timestamp) );

echo "===DONE===\n";

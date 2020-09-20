<?hh
/* Prototype  : string gmdate(string format [, long timestamp])
 * Description: Format a GMT date/time
 * Source code: ext/date/php_date.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing gmdate() : usage variation ***\n";

// Initialise all required variables
date_default_timezone_set('UTC');
$format = DATE_ISO8601;

echo "\n-- Testing gmdate() function with int 123456789000 to timestamp --\n";
$timestamp = 123456789000;
var_dump( gmdate($format, $timestamp) );

echo "\n-- Testing gmdate() function with int -123456789000 to timestamp --\n";
$timestamp = -123456789000;
var_dump( gmdate($format, $timestamp) );

echo "===DONE===\n";
}

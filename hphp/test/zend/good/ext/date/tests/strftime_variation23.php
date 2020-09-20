<?hh
/* Prototype  : string strftime(string format [, int timestamp])
 * Description: Format a local time/date according to locale settings
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing strftime() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
setlocale(LC_ALL, "en_US");
date_default_timezone_set("UTC");
$format = '%b %d %Y %H:%M:%S';

echo "\n-- Testing strftime() function with int 123456789000 to timestamp --\n";
$timestamp = 123456789000;
var_dump( strftime($format, $timestamp) );

echo "\n-- Testing strftime() function with int -123456789000 to timestamp --\n";
$timestamp = -123456789000;
var_dump( strftime($format, $timestamp) );

echo "===DONE===\n";
}

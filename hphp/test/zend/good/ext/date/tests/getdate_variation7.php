<?hh
/* Prototype  : array getdate([int timestamp])
 * Description: Get date/time information
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing getdate() : usage variation ***\n";
date_default_timezone_set("Asia/Calcutta");

echo "\n-- Testing getdate() function by passing int 123456789000 value to timestamp --\n";
$timestamp = 123456789000;
var_dump( getdate($timestamp) );

echo "\n-- Testing getdate() function by passing int -123456789000 value to timestamp --\n";
$timestamp = -123456789000;
var_dump( getdate($timestamp) );
echo "===DONE===\n";
}

<?php
/* Prototype  : string gmdate(string format [, long timestamp])
 * Description: Format a GMT date/time 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing gmdate() : error conditions ***\n";

// Initialise all required variables
date_default_timezone_set('UTC');
$format = DATE_ISO8601;
$timestamp = mktime(8, 8, 8, 8, 8, 2008);

// Zero arguments
echo "\n-- Testing gmdate() function with Zero arguments --\n";
var_dump( gmdate() );

//Test gmdate with one more than the expected number of arguments
echo "\n-- Testing gmdate() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( gmdate($format, $timestamp, $extra_arg) );

?>
===DONE===
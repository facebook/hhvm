<?php
/* Prototype  : int idate(string format [, int timestamp])
 * Description: Format a local time/date as integer 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing idate() : error conditions ***\n";

echo "\n-- Testing idate() function with Zero arguments --\n";
var_dump( idate() );

echo "\n-- Testing idate() function with more than expected no. of arguments --\n";
$format = '%b %d %Y %H:%M:%S';
$timestamp = gmmktime(8, 8, 8, 8, 8, 2008);
$extra_arg = 10;
var_dump( idate($format, $timestamp, $extra_arg) );

?>
===DONE===
<?php
/* Prototype  : string gmstrftime(string format [, int timestamp])
 * Description: Format a GMT/UCT time/date according to locale settings 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing gmstrftime() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing gmstrftime() function with Zero arguments --\n";
var_dump( gmstrftime() );

//Test gmstrftime with one more than the expected number of arguments
echo "\n-- Testing gmstrftime() function with more than expected no. of arguments --\n";
$format = '%b %d %Y %H:%M:%S';
$timestamp = gmmktime(8, 8, 8, 8, 8, 2008);
$extra_arg = 10;
var_dump( gmstrftime($format, $timestamp, $extra_arg) );

?>
===DONE===
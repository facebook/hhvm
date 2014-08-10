<?php
/* Prototype  : string gmstrftime(string format [, int timestamp])
 * Description: Format a GMT/UCT time/date according to locale settings 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing gmstrftime() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$timestamp = gmmktime(8, 8, 8, 8, 8, 2008);
setlocale(LC_ALL, "en_US");
date_default_timezone_set("Asia/Calcutta");

echo "\n-- Testing gmstrftime() function with  Abbreviated month name format %h --\n";
$format = "%h";
var_dump( gmstrftime($format) );
var_dump( gmstrftime($format, $timestamp) );

?>
===DONE===

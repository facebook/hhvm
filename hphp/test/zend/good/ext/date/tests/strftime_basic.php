<?php
/* Prototype  : string strftime(string format [, int timestamp])
 * Description: Format a local time/date according to locale settings 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing strftime() : basic functionality ***\n";

date_default_timezone_set("Asia/Calcutta");
// Initialise all required variables
$format = '%b %d %Y %H:%M:%S';
$timestamp = mktime(8, 8, 8, 8, 8, 2008);

// Calling strftime() with all possible arguments
var_dump( strftime($format, $timestamp) );

// Calling strftime() with mandatory arguments
var_dump( strftime($format) );

?>
===DONE===
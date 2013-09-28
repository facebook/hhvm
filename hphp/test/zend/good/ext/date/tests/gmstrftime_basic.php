<?php
/* Prototype  : string gmstrftime(string format [, int timestamp])
 * Description: Format a GMT/UCT time/date according to locale settings 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing gmstrftime() : basic functionality ***\n";

// Initialise all required variables
$format = '%b %d %Y %H:%M:%S';
$timestamp = gmmktime(8, 8, 8, 8, 8, 2008);

// Calling gmstrftime() with all possible arguments
var_dump( gmstrftime($format, $timestamp) );

// Calling gmstrftime() with mandatory arguments
var_dump( gmstrftime($format) );

?>
===DONE===
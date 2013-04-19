<?php
/* Prototype  : string gmdate(string format [, long timestamp])
 * Description: Format a GMT date/time 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing gmdate() : usage variation ***\n";

// Initialise all required variables
date_default_timezone_set('UTC');
$timestamp = mktime(8, 8, 8, 8, 8, 2008);

echo "\n-- Testing gmdate() function with ISO 8601 date format --\n";
var_dump( gmdate('c') );
var_dump( gmdate('c', $timestamp) );

echo "\n-- Testing gmdate() function with RFC 2822 date format --\n";
var_dump( gmdate('r') );
var_dump( gmdate('r', $timestamp) );

echo "\n-- Testing gmdate() function with seconds since Unix Epoch format --\n";
var_dump( gmdate('U') );
var_dump( gmdate('U', $timestamp) );

?>
===DONE===
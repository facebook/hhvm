<?php
/* Prototype  : bool syslog(int priority, string message)
 * Description: Generate a system log message 
 * Source code: ext/standard/syslog.c
 * Alias to functions: 
 */

echo "*** Testing syslog() : error conditions ***\n";


//Test syslog with one more than the expected number of arguments
echo "\n-- Testing syslog() function with more than expected no. of arguments --\n";
$priority = 10;
$message = 'string_val';
$extra_arg = 10;
var_dump( syslog($priority, $message, $extra_arg) );

// Testing syslog with one less than the expected number of arguments
echo "\n-- Testing syslog() function with less than expected no. of arguments --\n";
$priority = 10;
var_dump( syslog($priority) );

?>
===DONE===
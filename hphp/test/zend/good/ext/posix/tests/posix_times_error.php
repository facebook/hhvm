<?php
/* Prototype  : proto array posix_times(void)
 * Description: Get process times (POSIX.1, 4.5.2) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */

echo "*** Testing posix_times() : error conditions ***\n";

// One argument
echo "\n-- Testing posix_times() function with one argument --\n";
$extra_arg = 10;;
var_dump( posix_times($extra_arg) );

echo "Done";
?>
<?php
/* Prototype  : proto int posix_getgid(void)
 * Description: Get the current group id (POSIX.1, 4.2.1) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */

echo "*** Testing posix_getgid() : error conditions ***\n";

// One argument
echo "\n-- Testing posix_getgid() function with one argument --\n";
$extra_arg = 10;
var_dump( posix_getgid($extra_arg) );

echo "Done";
?>
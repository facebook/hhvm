<?php
/* Prototype  : proto int posix_getpid(void)
 * Description: Get the current process id (POSIX.1, 4.1.1) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */

echo "*** Testing posix_getpid() : error conditions ***\n";

// One argument
echo "\n-- Testing posix_getpid() function with one argument --\n";
$extra_arg = 10;
var_dump( posix_getpid($extra_arg) );

echo "Done";
?>
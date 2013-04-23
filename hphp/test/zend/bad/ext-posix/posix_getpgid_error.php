<?php
/* Prototype  : proto int posix_getpgid(void)
 * Description: Get the process group id of the specified process (This is not a POSIX function, but a SVR4ism, so we compile conditionally) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */

echo "*** Testing posix_getpgid() : error conditions ***\n";

echo "\n-- Testing posix_getpgid() function no arguments --\n";
var_dump( posix_getpgid() );

echo "\n-- Testing posix_getpgid() with one extra argument --\n";
$pid = 10;
$extra_arg = 20;
var_dump( posix_getpgid($pid, $extra_arg) );

echo "\n-- Testing posix_getpgid() with negative pid  --\n";
$pid = -99;
var_dump( posix_getpgid($pid) );

echo "Done";
?>
<?php
/* Prototype  : proto array posix_getpwuid(long uid)
 * Description: User database access (POSIX.1, 9.2.2) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */

echo "*** Testing posix_getpwuid() : error conditions ***\n";

echo "\n-- Testing posix_getpwuid() function with Zero arguments --\n";
var_dump( posix_getpwuid() );

echo "\n-- Testing posix_getpwuid() function with more than expected no. of arguments --\n";
$uid = posix_getuid();
$extra_arg = 10;
var_dump( posix_getpwuid($uid, $extra_arg) );

echo "\n-- Testing posix_getpwuid() function negative uid --\n";
$uid = -99;
var_dump( posix_getpwuid($uid) );

echo "Done";
?>
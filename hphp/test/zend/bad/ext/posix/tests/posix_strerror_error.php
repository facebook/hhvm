<?php
/* Prototype  : proto string posix_strerror(int errno)
 * Description: Retrieve the system error message associated with the given errno. 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */

echo "*** Testing posix_strerror() : error conditions ***\n";

echo "\n-- Testing posix_strerror() function with Zero arguments --\n";
var_dump( posix_strerror() );

echo "\n-- Testing posix_strerror() function with more than expected no. of arguments --\n";
$errno = posix_get_last_error();
$extra_arg = 10;
var_dump( posix_strerror($errno, $extra_arg) );

echo "\n-- Testing posix_strerror() function with invalid error number --\n";
$errno = -999;
echo gettype( posix_strerror($errno) )."\n";

echo "Done";
?>
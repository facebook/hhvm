<?php
/* Prototype  : proto array posix_getgrgid(long gid)
 * Description: Group database access (POSIX.1, 9.2.1) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */

echo "*** Testing posix_getgrgid() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing posix_getgrgid() function with Zero arguments --\n";
var_dump( posix_getgrgid() );

//Test posix_getgrgid with one more than the expected number of arguments
echo "\n-- Testing posix_getgrgid() function with more than expected no. of arguments --\n";

$extra_arg = 10;
$gid = 0;
var_dump( posix_getgrgid($gid, $extra_arg) );

echo "\n-- Testing posix_getgrgid() function with a negative group id --\n";
$gid = -999;
var_dump( posix_getgrgid($gid));

echo "Done";
?>
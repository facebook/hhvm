<?php
/* Prototype  : bool chdir(string $directory)
 * Description: Change the current directory 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass incorrect number of arguments to chdir() to test behaviour
 */

echo "*** Testing chdir() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing chdir() function with Zero arguments --\n";
var_dump( chdir() );

//Test chdir with one more than the expected number of arguments
echo "\n-- Testing chdir() function with more than expected no. of arguments --\n";
$directory = __FILE__;
$extra_arg = 10;
var_dump( chdir($directory, $extra_arg) );
?>
===DONE===
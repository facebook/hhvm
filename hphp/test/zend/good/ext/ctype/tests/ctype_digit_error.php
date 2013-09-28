<?php
/* Prototype  : bool ctype_digit(mixed $c)
 * Description: Checks for numeric character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass an incorrect number of arguments to ctype_digit() to test behaviour
 */

echo "*** Testing ctype_digit() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing ctype_digit() function with Zero arguments --\n";
var_dump( ctype_digit() );

//Test ctype_digit with one more than the expected number of arguments
echo "\n-- Testing ctype_digit() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
var_dump( ctype_digit($c, $extra_arg) );
?>
===DONE===
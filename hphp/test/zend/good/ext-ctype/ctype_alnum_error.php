<?php
/* Prototype  : bool ctype_alnum(mixed $c)
 * Description: Checks for alphanumeric character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass incorrect number of arguments to ctype_alnum() to test behaviour
 */

echo "*** Testing ctype_alnum() : error conditions ***\n";

$orig = setlocale(LC_CTYPE, "C"); 

// Zero arguments
echo "\n-- Testing ctype_alnum() function with Zero arguments --\n";
var_dump( ctype_alnum() );

//Test ctype_alnum with one more than the expected number of arguments
echo "\n-- Testing ctype_alnum() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
var_dump( ctype_alnum($c, $extra_arg) );

setlocale(LC_CTYPE, $orig);
?>
===DONE===
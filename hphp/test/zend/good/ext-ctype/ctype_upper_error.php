<?php
/* Prototype  : bool ctype_upper(mixed $c)
 * Description: Checks for uppercase character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass incorrect number of arguments to ctype_upper() to test behaviour
 */

echo "*** Testing ctype_upper() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing ctype_upper() function with Zero arguments --\n";
var_dump( ctype_upper() );

//Test ctype_upper with one more than the expected number of arguments
echo "\n-- Testing ctype_upper() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
var_dump( ctype_upper($c, $extra_arg) );
?>
===DONE===
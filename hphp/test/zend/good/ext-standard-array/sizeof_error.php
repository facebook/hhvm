<?php
/* Prototype  : int sizeof(mixed $var[, int $mode] )
 * Description: Counts an elements in an array. If Standard PHP Library is installed, 
 * it will return the properties of an object.
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: count()
 */

// Calling sizeof() with zero and more than expected arguments .

echo "*** Testing sizeof() : error conditions ***\n";

echo "-- Testing sizeof() with zero arguments --\n";
var_dump( sizeof() );
echo "-- Testing sizeof() function with more than two arguments under COUNT_NORMAL mode --\n";
$var = 100;
$extra_arg = 10;;
var_dump( sizeof($var, COUNT_NORMAL, $extra_arg) );
echo "-- Testing sizeof() function with more than two arguments under COUNT_RECURSIVE mode --\n";
var_dump( sizeof($var, COUNT_RECURSIVE, $extra_arg) );

echo "Done";
?>
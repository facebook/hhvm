<?php
/* Prototype  : bool uksort(array array_arg, string cmp_function)
 * Description: Sort an array by keys using a user-defined comparison function 
 * Source code: ext/standard/array.c
 * Alias to functions: 
 */

echo "*** Testing uksort() : error conditions ***\n";

echo "\n-- Testing uksort() function with more than expected no. of arguments --\n";
$array_arg = array(1, 2);
$cmp_function = 'string_val';
$extra_arg = 10;
var_dump( uksort($array_arg, $cmp_function, $extra_arg) );

echo "\n-- Testing uksort() function with less than expected no. of arguments --\n";
$array_arg = array(1, 2);
var_dump( uksort($array_arg) );

echo "\n-- Testing uksort() function with zero arguments --\n";
var_dump( uksort() );

?>
===DONE===
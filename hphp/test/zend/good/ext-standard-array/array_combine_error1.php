<?php
/* Prototype  : array array_combine(array $keys, array $values)
 * Description: Creates an array by using the elements of the first parameter as keys 
 *              and the elements of the second as the corresponding values 
 * Source code: ext/standard/array.c
*/

echo "*** Testing array_combine() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_combine() function with Zero arguments --\n";
var_dump( array_combine() );

//Test array_combine with one more than the expected number of arguments
echo "\n-- Testing array_combine() function with more than expected no. of arguments --\n";
$keys = array(1, 2);
$values = array(1, 2);
$extra_arg = 10;
var_dump( array_combine($keys,$values, $extra_arg) );

// Testing array_combine with one less than the expected number of arguments
echo "\n-- Testing array_combine() function with less than expected no. of arguments --\n";
$keys = array(1, 2);
var_dump( array_combine($keys) );

echo "Done";
?>
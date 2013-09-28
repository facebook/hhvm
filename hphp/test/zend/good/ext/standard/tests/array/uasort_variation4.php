<?php
/* Prototype  : bool uasort(array $array_arg, string $cmp_function)
 * Description: Sort an array with a user-defined comparison function and maintain index association 
 * Source code: ext/standard/array.c
*/

/*
* sorting different types of numeric arrays containing data of following type:
*  integer, octal, hexadecimal & float
*/

// comparison function
/* Prototype : int cmp_function(mixed $value1, mixed $value2)
 * Parameters : $value1 and $value2 - values to be compared
 * Return value : 0 - if both values are same
 *                1 - if value1 is greater than value2
 *               -1 - if value1 is less than value2
 * Description : compares value1 and value2
 */
function cmp_function($value1, $value2)
{
  if($value1 == $value2) {
    return 0;
  }
  else if($value1 > $value2) {
    return 1;
  }
  else {
    return -1;
  }
}

echo "*** Testing uasort() : different numeric arrays as 'array_arg' ***\n";

// Int array
$int_values = array(0 => 3, 1 => 2, 3 => 100, 4 => 150, 5 => 25, 6 => 350, 7 => 0, 8 => -3, 9 => -1200);
echo "-- Sorting Integer array --\n";
var_dump( uasort($int_values, 'cmp_function') );  // expecting: bool(true)
var_dump($int_values);

// Octal array
$octal_values = array(0 => 056, 1 => 023, 2 => 090, 3 => 015, 4 => -045, 5 => 01, 6 => -078);
echo "-- Sorting Octal array --\n";
var_dump( uasort($octal_values, 'cmp_function') );  // expecting: bool(true)
var_dump($octal_values);

// Hexadecimal array
$hex_values = array(0 => 0xAE, 1 => 0x2B, 2 => 0X10, 3 => -0xCF, 4 => 0X12, 5 => -0XF2);
echo "-- Sorting Hex array --\n";
var_dump( uasort($hex_values, 'cmp_function') );  // expecting: bool(true)
var_dump($hex_values);

// Float array
$float_values = array( 0 => 10.2, 1 => 2.4, 2 => -3.4, 3 => 0, 4 => 0.5, 5 => 7.3e3, 6 => -9.34E-2);
echo "-- Sorting Float array --\n";
var_dump( uasort($float_values, 'cmp_function') );  // expecting: bool(true)
var_dump($float_values);

// empty array
$empty_array = array();
echo "-- Sorting empty array --\n";
var_dump( uasort($empty_array, 'cmp_function') );  // expecting: bool(true)
var_dump($empty_array);

echo "Done"
?>
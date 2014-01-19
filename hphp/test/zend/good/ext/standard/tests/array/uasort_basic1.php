<?php
/* Prototype  : bool uasort(array $array_arg, string $cmp_function)
 * Description: Sort an array with a user-defined comparison function and maintain index association 
 * Source code: ext/standard/array.c
*/

echo "*** Testing uasort() : basic functionality ***\n";

// comparison function
/* Prototype : int cmp(mixed $value1, mixed $value2)
 * Parameters : $value1 and $value2 - values to be compared
 * Return value : 0 - if both values are same
 *                1 - if value1 is greater than value2
 *               -1 - if value1 is less than value2
 * Description : compares value1 and value2
 */
function cmp($value1, $value2)
{
  if($value1 == $value2) {
    return 0;
  }
  else if($value1 > $value2) {
    return 1;
  }
  else
    return -1;
}

// Int array with default keys
$int_values = array(1, 8, 9, 3, 2, 6, 7);
echo "-- Numeric array with default keys --\n";
var_dump( uasort($int_values, 'cmp') );
var_dump($int_values);

// String array with default keys
$string_values = array("This", "is", 'a', "test");
echo "-- String array with default keys --\n";
var_dump( uasort($string_values, 'cmp') );
var_dump($string_values);

// Associative array with numeric keys
$numeric_key_arg = array(1=> 1, 2 => 2, 3 => 7, 5 => 4, 4 => 9);
echo "-- Associative array with numeric keys --\n";
var_dump( uasort($numeric_key_arg, 'cmp') );
var_dump($numeric_key_arg);

// Associative array with string keys
$string_key_arg = array('one' => 4, 'two' => 2, 'three' => 1, 'four' => 10);
echo "-- Associative array with string keys --\n";
var_dump( uasort($string_key_arg, 'cmp') );
var_dump($string_key_arg);

echo "Done"
?>
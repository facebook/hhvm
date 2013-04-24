<?php
/* Prototype  : bool array_walk(array $input, string $funcname [, mixed $userdata])
 * Description: Apply a user function to every member of an array 
 * Source code: ext/standard/array.c
*/

/*
 * Testing array_walk() with following types of 'input' arrays:
 *    integer, float, string, bool, null, empty & mixed
*/

// callback function
/*
 * Prototype : print_value(mixed $value, int $key, int $count)
 * Parameters : $value - array entries(values)
 *              $key - keys in given input array
 *              $count - extra parameter used as an index
 * Description : prints the array values with keys and count value
 */
function print_value($value, $key, $count)
{
  echo  $count." : ".$key." ".$value."\n";
}

echo "*** Testing array_walk() : 'input' array with different values***\n";

// different arrays as input
$input_values = array(
  
       // integer values
/*1*/  array(1, 0, -10, 023, -041, 0x5A, 0X1F, -0x6E),
  
       // float value 
       array(3.4, 0.8, -2.9, 6.25e2, 8.20E-3),

       // string values
       array('Mango', "Apple", 'Orange', "Lemon"),

       // bool values
/*4*/  array(true, false, TRUE, FALSE),

       // null values
       array(null, NULL),

       // empty array
       array(),

       // binary array
       array(b"binary"),

       // mixed array
/*8*/  array(16, 8.345, "Fruits", true, null, FALSE, -98, 0.005, 'banana')
);

for($count = 0; $count < count($input_values); $count++) {
  echo "\n-- Iteration ".($count + 1)." --\n";
  var_dump( array_walk($input_values[$count], "print_value", $count+1));
}  
echo "Done"
?>
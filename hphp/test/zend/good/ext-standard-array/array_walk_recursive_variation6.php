<?php
/* Prototype  : bool array_walk_recursive(array $input, string $funcname [, mixed $userdata])
 * Description: Apply a user function to every member of an array 
 * Source code: ext/standard/array.c
*/

/*
 * Passing 'input' argument as an associative array 
 *    with Numeric & string keys
*/

echo "*** Testing array_walk_recursive() : 'input' as an associative array ***\n";

// callback functions
/* Prototype : for_numeric( int $value, int $key, int $user_data)
 * Parameters : $value - value from key/value pair of the array 
 *              $key - key from key/value pair of the array
 *              $user_data - data to be added to 'value'
 * Description : Function adds values with keys & user_data
 */
function for_numeric($value, $key, $user_data)
{
  // dump the input values to see if they are 
  // passed with correct type
  var_dump($key);
  var_dump($value);
  var_dump($user_data);
  echo "\n"; // new line to separate the output between each element
}

/* Prototype : for_string( string $value, string $key)
 * Parameters : $value - values in given input array
 *              $key - keys in given input array
 * Description : Function appends key to the value
 */
function for_string($value, $key)
{
  // dump the input values to see if they are 
  // passed with correct type
  var_dump($key);
  var_dump($value);
  echo "\n"; // new line to separate the output between each element
}

/* Prototype : for_mixed( mixed $value, mixed $key)
 * Parameters : $value - values in given input array
 *              $key - keys in given input array
 * Description : Function displays each element of an array with keys
 */
function for_mixed($value, $key)
{
  // dump the input values to see if they are 
  // passed with correct type
  var_dump($key);
  var_dump($value);
  echo "\n"; // new line to separate the output between each element
}

// Numeric keys
$input = array( 0 => array(1 => 25, 5 => 12, 0 => -80), 1 => array(-2 => 100, 5 => 30));
echo "-- Associative array with numeric keys --\n";
var_dump( array_walk_recursive($input, "for_numeric", 10));

// String keys
$input = array( "a" => "Apple", 'z' => array('b' => 'Bananna', "c" => "carrot"), 'x' => array('o' => "Orange"));
echo "-- Associative array with string keys --\n";
var_dump( array_walk_recursive($input, "for_string"));

// binary key
$input = array( b"a" => "Apple", b"b" => "Banana");
echo "-- Associative array with binary keys --\n";
var_dump( array_walk_recursive($input, "for_string"));

// Mixed keys - numeric/string
$input = array( 0 => array(0 => 1, 1 => 2), "x" => array("a" => "Apple", "b" => "Banana"), 2 =>3);
echo "-- Associative array with numeric/string keys --\n";
var_dump( array_walk_recursive($input, "for_mixed"));

echo "Done"
?>
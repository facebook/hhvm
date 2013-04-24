<?php
/* Prototype  : mixed array_sum(array $input)
 * Description: Returns the sum of the array entries 
 * Source code: ext/standard/array.c
*/

/*
* Testing array_sum() with array having other than numeric entries
*    strings, bool, null, subarrays & objects
*/

echo "*** Testing array_sum() : array with unexpected entries ***\n";

// empty array
$input = array();
echo "-- empty array --\n";
var_dump( array_sum($input) );

// string array
$input = array('Apple', 'Banana', 'Carrot', 'Mango', 'Orange');
echo "-- array with string values --\n";
var_dump( array_sum($input) );

// bool array
$input = array( true, true, false, true, false);
echo "-- array with bool values --\n";
var_dump( array_sum($input) );

// array with null entry
$input = array(null, NULL);
echo "-- array with null values --\n";
var_dump( array_sum($input) );

// array with subarray
$input = array(
  array(1, 2),
  array(),
  array(0)
);
echo "-- array with subarrays --\n";
var_dump( array_sum($input) );

class MyClass
{
  public $value;
  public function __construct($value)
  {
    $this->value = $value;
  }
}
// array of objects
$input = array(
  new MyClass(2),
  new MyClass(5),
  new MyClass(10),
  new MyClass(0)
);
echo "-- array with object values --\n";
var_dump( array_sum($input) );

// Mixed values
$input = array( 5, -8, 7.2, -1.2, "10", "apple", 'Mango', true, false, null, NULL, array( array(1,2), array(0), array()));
echo "-- array with mixed values --\n";
var_dump( array_sum($input) );
echo "Done"
?>
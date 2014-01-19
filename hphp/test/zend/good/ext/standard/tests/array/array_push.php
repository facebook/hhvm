<?php

/* Prototype: int array_push( array &array );
 * Description: Push one or more elements onto the end of array
 and returns the new number of elements in the array.
 */

$empty_array = array();
$number = 5;
$str = "abc";


/* Various combinations of arrays to be used for the test */
$mixed_array = array(
  array(),
  array( 1,2,3,4,5,6,7,8,9 ),
  array( "One", "_Two", "Three", "Four", "Five" ),
  array( 6, "six", 7, "seven", 8, "eight", 9, "nine" ),
  array( "a" => "aaa", "A" => "AAA", "c" => "ccc", "d" => "ddd", "e" => "eee" ),
  array( "1" => "one", "2" => "two", "3" => "three", "4" => "four", "5" => "five" ),
  array( 1 => "one", 2 => "two", 3 => 7, 4 => "four", 5 => "five" ),
  array( "f" => "fff", "1" => "one", 4 => 6, "" => "blank", 2.4 => "float", "F" => "FFF",
         "blank" => "", 3.7 => 3.7, 5.4 => 7, 6 => 8.6, '5' => "Five", "4name" => "jonny", "a" => NULL, NULL => 3 ),
  array( 12, "name", 'age', '45' ),
  array( array("oNe", "tWo", 4), array(10, 20, 30, 40, 50), array() ),
  array( "one" => 1, "one" => 2, "three" => 3, 3, 4, 3 => 33, 4 => 44, 5, 6,
          5.4 => 54, 5.7 => 57, "5.4" => 554, "5.7" => 557 )
);

/* Error Conditions */
echo "\n*** Testing Error Conditions ***\n";

/* Zero argument  */
var_dump( array_push() );

/* Scalar argument */
var_dump( array_push($number, 22) );

/* String argument */
var_dump( array_push($str, 22) );

/* Invalid Number of arguments */
var_dump( array_push($mixed_array[1],1,2) );

/* Empty Array as argument */
var_dump( array_push($empty_array, 2) );


/* Loop to test normal functionality with different arrays inputs */
echo "\n*** Testing with various array inputs ***\n";

$counter = 1;
foreach( $mixed_array as $sub_array )
{ 
 echo "\n-- Input Array for Iteration $counter is --\n";
 print_r( $sub_array );
 echo "\nOutput after push is :\n";
 var_dump( array_push($sub_array, 22, "abc") );
 $counter++;
} 

/* Checking for return value and the new array formed from push operation */
echo "\n*** Checking for return value and the new array formed from push operation ***\n";
var_dump( array_push($mixed_array[2], 22, 33, "44") );
var_dump( $mixed_array[2] );

echo"\nDone";
?>
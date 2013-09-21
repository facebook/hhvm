<?php
/* Prototype & Usage: 
   mixed key ( array &$array ) -> returns the index element of the current array position
   mixed current ( array &$array ) -> returns the current element in the array
   mixed next ( array &$array ) -> similar to current() but advances the internal pointer to next element
   mixed reset ( array &$array ) -> Reset the internal pointer to first element
*/

$basic_arrays = array (
  array(0),  // array with element as 0
  array(1),  // array with single element
  array(1,2, 3, -1, -2, -3),               // array of integers
  array(1.1, 2.2, 3.3, -1.1, -2.2, -3.3),  // array of floats
  array('a', 'b', 'c', "ab", "ac", "ad"),  // string array
  array("a" => "apple", "b" => "book", "c" => "cook"),  // associative array
  array('d' => 'drink', 'p' => 'port', 's' => 'set'),   // another associative array
  array(1 => 'One', 2 => 'two', 3 => "three")           // associative array with key as integers
);
            
$varient_arrays = array (
   array(),    // empty array
   array(""),  // array with null string
   array(NULL),// array with NULL 
   array(null),// array with null
   array(NULL, true, null, "", 1), // mixed array
   array(-1.5 => "test", -2 => "rest", 2.5 => "two", 
         "" => "string", 0 => "zero", "" => "" ) // mixed array
);  

echo "*** Testing basic operations ***\n";
$loop_count = 1;
foreach ($basic_arrays as $sub_array )  {
  echo "-- Iteration $loop_count --\n";
  $loop_count++;
  $c = count ($sub_array);
  $c++; // increment by one to create the situation of accessing beyond array size
  while ( $c ) {
    var_dump( current($sub_array)); // current element
    var_dump( key($sub_array) );    // key of the current element
    var_dump( next($sub_array) );   // move to next element
    $c --;
  }
  var_dump( reset($sub_array) );    // reset the internal pointer to first element
  var_dump( key($sub_array) );      // access the array after reset
  var_dump( $sub_array );           // dump the array to see that its intact

  echo "\n";
}

echo "\n*** Testing possible variations ***\n";
$loop_count = 1;
foreach ($varient_arrays as $sub_array )  {
  echo "-- Iteration $loop_count --\n";
  $loop_count++;
  $c = count ($sub_array);
  $c++; // increment by one to create the situation of accessing beyond array size
  while ( $c ) {
    var_dump( current($sub_array)); // current element
    var_dump( key($sub_array) );    // key of the current element
    var_dump( next($sub_array) );   // move to next element
    $c --;
  }
  var_dump( reset($sub_array) );    // reset the internal pointer to first element
  var_dump( key($sub_array) );      // access the array after reset
  var_dump( $sub_array );           // dump the array to see that its intact
  echo "\n";
}

/*test these functions on array which is already unset */ 
echo "\n-- Testing variation: when array is unset --\n";
$unset_array = array (1);
unset($unset_array);

var_dump( current($unset_array) );
var_dump( key($unset_array) );
var_dump( next($unset_array) );
var_dump( reset($unset_array) );


echo "\n*** Testing error conditions ***\n";
//Zero argument, expected 1 argument
var_dump( key() );
var_dump( current() );
var_dump( reset() );
var_dump( next() );

// args more than expected, expected 1 argument
$temp_array = array(1);
var_dump( key($temp_array, $temp_array) );
var_dump( current($temp_array, $temp_array) );
var_dump( reset($temp_array, $temp_array) );
var_dump( next($temp_array, $temp_array) );

// invalid args type, valid argument: array 
$int_var = 1;
$float_var = 1.5;
$string = "string";
var_dump( key($int_var) );
var_dump( key($float_var) );
var_dump( key($string) );

var_dump( current($int_var) );
var_dump( current($float_var) );
var_dump( current($string) );

var_dump( next($int_var) );
var_dump( next($float_var) );
var_dump( next($string) );

var_dump( reset($int_var) );
var_dump( reset($float_var) );
var_dump( reset($string) );

echo "Done\n";
?>
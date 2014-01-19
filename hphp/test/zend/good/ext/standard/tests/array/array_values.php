<?php
/* Prototype: array array_values ( array $input );
   Discription: array_values() returns all the values from the input array 
                and indexes numerically the array
*/

echo "*** Testing array_values() on basic array ***\n"; 
$basic_arr = array( 1, 2, 2.0, "asdasd", array(1,2,3) );
var_dump( array_values($basic_arr) );

echo "\n*** Testing array_values() on various arrays ***";
$arrays = array (
  array(), 
  array(0),
  array(-1),
  array( array() ),
  array("Hello"),
  array(""),  
  array("", array()),
  array(1,2,3), 
  array(1,2,3, array()),
  array(1,2,3, array(4,6)),
  array("a" => 1, "b" => 2, "c" =>3),
  array(0 => 0, 1 => 1, 2 => 2),  
  array(TRUE, FALSE, NULL, true, false, null, "TRUE", "FALSE",
        "NULL", "\x000", "\000"),
  array("Hi" => 1, "Hello" => 2, "World" => 3),
  array("a" => "abcd", "a" => "", "ab" => -6, "cd" => -0.5 ),
  array(0 => array(), 1=> array(0), 2 => array(1), ""=> array(), ""=>"" )
);

$i = 0;
/* loop through to test array_values() with different arrays given above */
foreach ($arrays as $array) {
  echo "\n-- Iteration $i --\n";
  var_dump( array_values($array) );
  $i++;
}

echo "Done\n";
?>
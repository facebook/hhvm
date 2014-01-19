<?php
/* Prototype: array array_merge(array $array1 [, array $array2 [, array $...]]);
   Description: Merge one or more arrays
*/

echo "\n*** Testing array_merge() basic functionality ***";
$begin_array = array(
  array(),
  array( 1 => "string"),
  array( "" => "string"),
  array( -2.44444 => 12),
  array( "a" => 1, "b" => -2.344, "b" => "string", "c" => NULL,	"d" => -2.344),
  array( 4 => 1, 3 => -2.344, "3" => "string", "2" => NULL,1 => -2.344),
  array( NULL, 1.23 => "Hi", "string" => "hello", 
  array("" => "World", "-2.34" => "a", "0" => "b"))
);

$end_array   = array(
  array(),
  array( 1 => "string"),
  array( "" => "string"),
  array( -2.44444 => 12),
  array( "a" => 1, "b" => -2.344, "b" => "string", "c" => NULL, "d" => -2.344),
  array( 4 => 1, 3 => -2.344, "3" => "string", "2" => NULL, 1=> -2.344), 
  array( NULL, 1.23 => "Hi", "string" => "hello", 
         array("" => "World", "-2.34" => "a", "0" => "b"))
);

/* loop through to merge two arrays */
$count_outer = 0;
foreach($begin_array as $first) {
  echo "\n\n--- Iteration $count_outer ---";
  $count_inner = 0;
  foreach($end_array as $second) {
    echo "\n-- Inner iteration $count_inner of Iteration $count_outer --\n";
    $result = array_merge($first, $second);
    print_r($result);
    $count_inner++;
  }			
  $count_outer++;
}


echo "\n*** Testing array_merge() with three or more arrays ***\n";
var_dump( array_merge( $end_array[0], 
                       $end_array[5], 
                       $end_array[4],
                       $end_array[6]
                     )
        );

var_dump( array_merge( $end_array[0], 
                       $end_array[5], 
                       array("array on fly"), 
                       array("nullarray" => array())
                     )
        );


echo "\n*** Testing single array argument ***\n";
/* Empty array */
var_dump(array_merge(array())); 

/* associative array with string keys, which will not be re-indexed */
var_dump(array_merge($begin_array[4]));

/* associative array with numeric keys, which will be re-indexed */
var_dump(array_merge($begin_array[5]));

/* associative array with mixed keys and sub-array as element */
var_dump(array_merge($begin_array[6]));

echo "\n*** Testing array_merge() with typecasting non-array to array ***\n";
var_dump(array_merge($begin_array[4], (array)"type1", (array)10, (array)12.34));

echo "\n*** Testing error conditions ***";
/* Invalid arguments */
var_dump(array_merge());
var_dump(array_merge(100, 200));
var_dump(array_merge($begin_array[0], $begin_array[1], 100));
var_dump(array_merge($begin_array[0], $begin_array[1], $arr4));

echo "Done\n";
?> 
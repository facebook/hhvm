<?php
/*
 * Prototype  : mixed array_search ( mixed $needle, array $haystack [, bool $strict] )
 * Description: Searches haystack for needle and returns the key if it is found in the array, FALSE otherwise
 * Source Code: ext/standard/array.c
*/

/* Test array_search() with different possible haystack values */

echo "*** Testing array_search() with different haystack values ***\n";

$misc_array = array (
  'a',
  'key' =>'d',
  3,
  ".001" =>-67, 
  "-.051" =>"k",
  0.091 =>"-.08",
  "e" =>"5", 
  "y" =>NULL,
  NULL =>"",
  0,
  TRUE,
  FALSE,
  -27.39999999999,
  " ",
  "abcd\x00abcd\x00\abcd\x00abcdefghij",
  "abcd\nabcd\tabcd\rabcd\0abcd"
);
$array_type = array(TRUE, FALSE, 1, 0, -1, "1", "0", "-1", NULL, array(), "PHP", "");
/* loop to do loose and strict type check of elements in
   $array_type on elements in $misc_array using array_search();
   checking PHP type comparison tables
*/
$counter = 1;
foreach($array_type as $type) {
  echo "-- Iteration $counter --\n";
  //loose type checking
  var_dump( array_search($type,$misc_array ) );  
  //strict type checking
  var_dump( array_search($type,$misc_array,true) );  
  //loose type checking
  var_dump( array_search($type,$misc_array,false) );  
  $counter++;
}

echo "Done\n";
?>
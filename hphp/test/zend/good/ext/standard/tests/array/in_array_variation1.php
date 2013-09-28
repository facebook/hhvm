<?php
/*
 * Prototype  : bool in_array ( mixed $needle, array $haystack [, bool $strict] )
 * Description: Searches haystack for needle and returns TRUE  
 *              if it is found in the array, FALSE otherwise.
 * Source Code: ext/standard/array.c
*/

/* Test in_array() with different possible needle values */

echo "*** Testing in_array() with different needle values ***\n";
$arrays = array (
  array(0),
  array("a" => "A", 2 => "B", "C" => 3, 4 => 4, "one" => 1, "" => NULL, "b", "ab", "abcd"),
  array(4, array(1, 2 => 3), "one" => 1, "5" => 5 ),
  array(-1, -2, -3, -4, -2.989888, "-0.005" => "neg0.005", 2.0 => "float2", "-.9" => -.9),
  array(TRUE, FALSE),
  array("", array()),
  array("abcd\x00abcd\x00abcd"),
  array("abcd\tabcd\nabcd\rabcd\0abcdefghij") 
);

$array_compare = array (
  4,
  "4",
  4.00,
  "b",
  "5",
  -2,
  -2.0,
  -2.98989,
  "-.9",
  "True",
  "",
  array(),
  NULL,
  "ab",
  "abcd",
  0.0,
  -0,
  "abcd\x00abcd\x00abcd"
);
/* loop to check if elements in $array_compare exist in $arrays
   using in_array() */
$counter = 1;
foreach($arrays as $array) {
  foreach($array_compare as $compare) {
    echo "-- Iteration $counter --\n";
    //strict option OFF
    var_dump(in_array($compare,$array));  
    //strict option ON
    var_dump(in_array($compare,$array,TRUE));  
    //strict option OFF
    var_dump(in_array($compare,$array,FALSE));  
    $counter++;
 }
}
		
echo "Done\n";
?>
<?php
/* Prototype  : string ucwords ( string $str )
 * Description: Uppercase the first character of each word in a string
 * Source code: ext/standard/string.c
*/

echo "*** Testing ucwords() : basic functionality ***\n";

// lines with different whitespace charecter
$str_array = array(
 "testing ucwords",
 'testing ucwords',
 'testing\tucwords',
 "testing\tucwords",
 "testing\nucwords",
 'testing\nucwords',
 "testing\vucwords",
 'testing\vucwords',
 "testing",
 'testing',
 ' testing',
 " testing",
 "testing  ucwords",
 'testing  ucwords',
 'testing\rucwords',
 "testing\rucwords",
 'testing\fucwords',
 "testing\fucwords"
);

// loop through the $strings array to test ucwords on each element 
$iteration = 1;
for($index = 0; $index < count($str_array); $index++) {
  echo "-- Iteration $iteration --\n";
  var_dump( ucwords($str_array[$index]) );
  $iteration++;
}

echo "Done\n";
?>
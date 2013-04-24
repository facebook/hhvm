<?php
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays of numeric data to test how natcasesort re-orders the array
 */

echo "*** Testing natcasesort() : usage variation ***\n";

$inputs = array (

  // negative/positive integers array
  array(11, -11, 21, -21, 31, -31, 0, 41, -41),

  // float value array
  array(10.5, -10.5, 10.5e2, 10.6E-2, .5, .01, -.1),

  // mixed value array
  array(.0001, .0021, -.01, -1, 0, .09, 2, -.9, 10.6E-2, -10.6E-2, 33),
 
  // array values contains minimum and maximum ranges
  array(2147483647, 2147483648, -2147483647, -2147483648, -0, 0, -2147483649)
);

$iterator = 1;
foreach ($inputs as $array_arg) {
	echo "\n-- Iteration $iterator --\n";
	var_dump(natcasesort($array_arg));
	var_dump($array_arg);
}

echo "Done";
?>

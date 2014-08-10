<?php
/* Prototype  : bool usort(array $array_arg, string $cmp_function)
 * Description: Sort an array by values using a user-defined comparison function 
 * Source code: ext/standard/array.c
 */

/*
 * Pass an unknown comparison function to usort() to test behaviour.
 * Pass incorrect number of arguments and an unknown function to test which error
 * is generated.
 */

echo "*** Testing usort() : error conditions ***\n";

function cmp($value1, $value2)
{
  if($value1 == $value2) {
    return 0;
  }
  else if($value1 > $value2) {
    return 1;
  }
  else {
    return -1;
  }
}

// Initialize 'array_arg' 
$array_arg = array(0 => 1, 1 => 10, 2 => 'string', 3 => 3, 4 => 2, 5 => 100, 6 => 25);
$extra_arg = 10;

// With non existent comparison function
echo "\n-- Testing usort() function with non-existent compare function --\n";
var_dump( usort($array_arg, 'non_existent') );

// With non existent comparison function and extra argument
echo "\n-- Testing usort() function with non-existent compare function and extra argument --\n";
var_dump( usort($array_arg, 'non_existent', $extra_arg) );
?>
===DONE===

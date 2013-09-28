<?php
/* Prototype  : string vsprintf(string format, array args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vsprintf() when different int formats and int values are passed to
 * the '$format' and '$args' arguments of the function
*/

echo "*** Testing vsprintf() : int formats with int values ***\n";


// defining array of int formats
$formats = array(
  "%d",
  "%+d %-d %D",
  "%ld %Ld, %4d %-4d",
  "%10.4d %-10.4d %04d %04.4d",
  "%'#2d %'2d %'$2d %'_2d",
  "%d %d %d %d",
  "% %%d d%",
  '%3$d %4$d %1$d %2$d'
);

// Arrays of int values for the format defined in $format.
// Each sub array contains int values which correspond to each format string in $format
$args_array = array(
  array(0),
  array(-1, 1, +22),
  array(2147483647, -2147483648, +2147483640, -2147483640),
  array(123456, 12345678, -1234567, 1234567),
  array(111, 2222, 333333, 44444444),
  array(0x123b, 0xfAb, 0123, 01293),
  array(1234, -5678, 2345),
  array(3, 4, 1, 2)

);

// looping to test vsprintf() with different int formats from the above $format array
// and with int values from the above $args_array array
$counter = 1;
foreach($formats as $format) {
  echo "\n-- Iteration $counter --\n";   
  var_dump( vsprintf($format, $args_array[$counter-1]) );
  $counter++;
}

echo "Done";
?>

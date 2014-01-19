<?php
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/

/*
* Test vfprintf() for char formats with an array of chars passed to the function
*/

echo "*** Testing vfprintf() : char formats with char values ***\n";


// defining array of char formats
$formats = array(
  "%c",
  "%+c %-c %C",
  "%lc %Lc, %4c %-4c",
  "%10.4c %-10.4c %04c %04.4c",
  "%'#2c %'2c %'$2c %'_2c",
  "%c %c %c %c",
  "% %%c c%",
  '%3$c %4$c %1$c %2$c'
);

// Arrays of char values for the format defined in $format.
// Each sub array contains char values which correspond to each format string in $format
$args_array = array(
  array(0),
  array('c', 67, 68),
  array(' ', " ", -67, +67),
  array(97, -97, 98, +98),
  array(97, -97, 98, +98),
  array(0x123b, 0xfAb, 0123, 01293),
  array(38, -1234, 2345),
  array(67, 68, 65, 66)

);

/* creating dumping file */
$data_file = dirname(__FILE__) . '/vfprintf_variation9.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

// looping to test vfprintf() with different char formats from the above $format array
// and with char values from the above $args_array array
$counter = 1;
foreach($formats as $format) {
  fprintf($fp, "\n-- Iteration %d --\n",$counter);
  vfprintf($fp, $format, $args_array[$counter-1]);
  $counter++;
}

fclose($fp);
print_r(file_get_contents($data_file));
echo "\n";

unlink($data_file);
 
?>
===DONE===
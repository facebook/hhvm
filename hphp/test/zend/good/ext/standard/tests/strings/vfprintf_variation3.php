<?hh
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vfprintf() when different int formats and int values are passed to
 * the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : int formats with int values ***\n";


// defining array of int formats
$formats = varray[
  "%d",
  "%+d %-d %D",
  "%ld %Ld, %4d %-4d",
  "%10.4d %-10.4d %04d %04.4d",
  "%'#2d %'2d %'$2d %'_2d",
  "%d %d %d %d",
  "% %%d d%",
  '%3$d %4$d %1$d %2$d'
];

// Arrays of int values for the format defined in $format.
// Each sub array contains int values which correspond to each format string in $format
$args_array = varray[
  varray[0],
  varray[-1, 1, +22],
  varray[2147483647, -2147483648, +2147483640, -2147483640],
  varray[123456, 12345678, -1234567, 1234567],
  varray[111, 2222, 333333, 44444444],
  varray[0x123b, 0xfAb, 0123, 012],
  varray[1234, -5678, 2345],
  varray[3, 4, 1, 2]

];

// looping to test vfprintf() with different int formats from the above $format array
// and with int values from the above $args_array array

/* creating dumping file */
$data_file = sys_get_temp_dir().'/'.'vfprintf_variation3.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

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

echo "===DONE===\n";
}

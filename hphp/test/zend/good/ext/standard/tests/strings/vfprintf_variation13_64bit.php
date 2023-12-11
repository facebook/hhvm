<?hh
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vfprintf() when different hexa formats and hexa values are passed to
 * the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : hexa formats with hexa values ***\n";

// defining array of different hexa formats
$formats = vec[
  "%x",
  "%+x %-x %X",
  "%lx %Lx, %4x %-4x",
  "%10.4x %-10.4x %04x %04.4x",
  "%'#2x %'2x %'$2x %'_2x",
  "%x %x %x %x",
  "% %%x x%",
  '%3$x %4$x %1$x %2$x'
];

// Arrays of hexa values for the format defined in $format.
// Each sub array contains hexa values which correspond to each format string in $format
$args_array = vec[
  vec[0x0],
  vec[-0x1, 0x1, +0x22],
  vec[0x7FFFFFFF, -0x7fffffff, +0x7000000, -0x80000000],
  vec[123456, 12345678, -1234567, 1234567],
  vec[1, 0x2222, 0333333, -0x44444444],
  vec[0x123b, 0xfAb, "0xaxz", 012],
  vec[0x1234, 0x34, 0x2ff],
  vec[0x3, 0x4, 0x1, 0x2]

];

/* creating dumping file */
$data_file = sys_get_temp_dir().'/'.'vfprintf_variation13_64bit.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

// looping to test vfprintf() with different char octal from the above $format array
// and with octal values from the above $args_array array
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

<?hh
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vfprintf() when different octal formats and octal values are passed to
 * the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : octal formats with octal values ***\n";

// defining array of octal formats
$formats = vec[
  "%o",
  "%+o %-o %O",
  "%lo %Lo, %4o %-4o",
  "%10.4o %-10.4o %04o %04.4o",
  "%'#2o %'2o %'$2o %'_2o",
  "%o %o %o %o",
  "%% %%o %10 o%",
  '%3$o %4$o %1$o %2$o'
];

// Arrays of octal values for the format defined in $format.
// Each sub array contains octal values which correspond to each format string in $format
$args_array = vec[
  vec[00],
  vec[-01, 01, +022],
  vec[-020000000000, 020000000000, 017777777777, -017777777777],
  vec[0123456, 01234567, -01234567, 01234567],
  vec[0111, 02222, -0333333, -044444444],
  vec[0x123b, 0xfAb, 0123, 012],
  vec[01234, 0567, -01234, 02345],
  vec[03, 04, 01, 02]

];

/* creating dumping file */
$data_file = sys_get_temp_dir().'/'.'vfprintf_variation11_64bit.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

// looping to test vfprintf() with different octal formats from the above $formats array
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

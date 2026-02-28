<?hh
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vfprintf() when different unsigned formats and unsigned values
 * are passed to the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : unsigned formats and unsigned values ***\n";

// defining array of unsigned formats
$formats = vec[
  '%u %+u %-u', 
  '%lu %Lu %4u %-4u',
  '%10.4u %-10.4u %.4u', 
  '%\'#2u %\'2u %\'$2u %\'_2u',
  '%3$u %4$u %1$u %2$u'
];

// Arrays of unsigned values for the format defined in $format.
// Each sub array contains unsigned values which correspond to each format string in $format
$args_array = vec[
  vec[1234567, 01234567, 0 ],
  vec[12345678900, 12345678900, 1234, 12345],
  vec["1234000", 10.1234567e10, 1.2e2],
  vec[1, 0, 00, "10_"],
  vec[3, 4, 1, 2]
];

/* creating dumping file */
$data_file = sys_get_temp_dir().'/'.'vfprintf_variation15_64bit.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

// looping to test vfprintf() with different unsigned formats from the above $format array
// and with signed and other types of  values from the above $args_array array
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

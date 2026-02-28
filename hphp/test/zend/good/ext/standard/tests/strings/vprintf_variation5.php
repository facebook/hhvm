<?hh
/* Prototype  : string vprintf(string format, array args)
 * Description: Output a formatted string
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vprintf() when different float formats and float values are passed to
 * the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : int formats with float values ***\n";


// defining array of float formats
$formats = vec[
  "%f",
  "%+f %-f %F",
  "%lf %Lf, %4f %-4f",
  "%10.4f %-10.4F %04f %04.4f",
  "%'#2f %'2f %'$2f %'_2f",
  "%f %f %f %f",
  "% %%f f%",
  '%3$f %4$f %1$f %2$f'
];

// Arrays of float values for the format defined in $format.
// Each sub array contains float values which correspond to each format string in $format
$args_array = vec[
  vec[0.0],
  vec[-0.1, +0.1, +10.0000006],
  vec[2147483649, -2147483647, +2147483640, -2147483640],
  vec[2e5, 2e-5, -2e5, -2e-5],
  vec[0.2E5, -0.2e40, 0.2E-20, 0.2E+20],
  vec[0x123b, 0xfAb, 0123, 012],
  vec[1234.1234, -5678.5678, 2345.2345],
  vec[3.33, 4.44, 1.11, 2.22]

];

// looping to test vprintf() with different float formats from the above $format array
// and with float values from the above $args_array array
$counter = 1;
foreach($formats as $format) {
  echo "\n-- Iteration $counter --\n";
  $result = vprintf($format, $args_array[$counter-1]);
  echo "\n";
  var_dump($result);
  $counter++;
}

echo "===DONE===\n";
}

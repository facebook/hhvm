<?hh
/* Prototype  : string vsprintf(string format, array args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vsprintf() when different scientific formats and scientific values
 * are passed to the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vsprintf() : scientific formats and scientific values ***\n";

// defining array of scientific formats
$formats = vec[
  '%e %+e %-e', 
  '%le %Le %4e %-4e',
  '%10.4e %-10.4e %.4e', 
  '%\'#20e %\'20e %\'$20e %\'_20e',
  '%3$e %4$e %1$e %2$e'
];

// Arrays of scientific values for the format defined in $format.
// Each sub array contains scientific values which correspond to each format string in $format
$args_array = vec[
  vec[0, 1e0, "10e2" ],
  vec[2.2e2, 10e10, 1000e-2, 1000e7],
  vec[-22e12, 10e20, 1.2e2],
  vec[1e1, +1e2, -1e3, "1e2_"],
  vec[3e3, 4e3, 1e3, 2e3]
];
 
// looping to test vsprintf() with different scientific formats from the above $format array
// and with signed and other types of  values from the above $args_array array
$counter = 1;
foreach($formats as $format) {
  echo "\n-- Iteration $counter --\n";
  var_dump( vsprintf($format, $args_array[$counter-1]) );
  $counter++;
}

echo "Done";
}

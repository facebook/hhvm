<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : float formats with float values ***\n";

// array of float type values

$float_values = varray [
-2147483649, // float value
  2147483648,  // float value
  -0x80000001, // float value, beyond max negative int
  0x800000001, // float value, beyond max positive int
  020000000001, // float value, beyond max positive int
  -020000000001, // float value, beyond max negative int
  0.0,
  -0.1,
  10.0000000000000000005,
  10.5e+5,
  1e5,
  -1e5,
  1e-5,
  -1e-5,
  1e+5,
  -1e+5,
  1E5,
  -1E5,
  1E+5,
  -1E+5,
  1E-5,
  -1E-5,
  .5e+7,
  -.5e+7,
  .6e-19,
  -.6e-19,
  .05E+44,
  -.05E+44,
  .0034E-30,
  -.0034E-30
];

// various float formats
$float_formats = vec[
  "%f", "%hf", "%lf",
  "%Lf", " %f", "%f ",
  "\t%f", "\n%f", "%4f",
  "%30f", "%[0-9]", "%*f",
];

$count = 1;
foreach($float_values as $float_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($float_formats as $format) {
    var_dump( sprintf($format, $float_value) );
  }
  $count++;
};

echo "Done";
}

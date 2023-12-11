<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : scientific formats with float values ***\n";

// array of float values 
$float_values = vec[
  -2147483649,
  2147483648,
  -0x80000001, // float value, beyond max negative int
  0x800000001, // float value, beyond max positive int
  020000000001, // float value, beyond max positive int
  -020000000001, // float value, beyond max negative int
  0.0,
  -0.1,
  1.0,
  1e5,
  -1e5,
  -1e5,
  +1e5,
  1e+5,
  -1e-5,
  1E8,
  -1E9,
  10.0000000000000000005,
  10.5e+5
];

// array of scientific formats
$scientific_formats = vec[
  "%e", "%he", "%le",
  "%Le", " %e", "%e ",
  "\t%e", "\n%e", "%4e",
  "%30e", "%[0-1]", "%*e"
];


$count = 1;
foreach($float_values as $float_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($scientific_formats as $format) {
    var_dump( sprintf($format, $float_value) );
  }
  $count++;
};

echo "Done";
}

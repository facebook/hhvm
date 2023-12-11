<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : hexa formats with float values ***\n";

// array of float values 
$float_values = vec[
  2147483647,
  0x800000001, // float value, beyond max positive int
  020000000001, // float value, beyond max positive int
  0.0,
  -0.1,
  1.0,
  1e5,
  -1e6,
  1E8,
  -1E9,
  10.0000000000000000005,
  10.5e+5
];

// array of hexa formats
$hexa_formats = vec[  
  "%x", "%xx", "%lx", 
  "%Lx", " %x", "%x ",
  "\t%x", "\n%x", "%4x",
  "%30x", "%[0-9A-Fa-f]", "%*x"
];

$count = 1;
foreach($float_values as $float_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($hexa_formats as $format) {
    // with two arguments
    var_dump( sprintf($format, $float_value) );
  }
  $count++;
};

echo "Done";
}

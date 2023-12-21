<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : float formats with arrays ***\n";

// array of array types
$array_values = vec[
  vec[],
  vec[0],
  vec[1],
  vec[NULL],
  vec[null],
  vec["string"],
  vec[true],
  vec[TRUE],
  vec[false],
  vec[FALSE],
  vec[1,2,3,4],
  vec["123.456abc"],
  vec['123.456abc'],
  dict[1 => "One", "two" => 2]
];

// various float formats
$float_formats = vec[
  "%f", "%hf", "%lf", 
  "%Lf", " %f", "%f ", 
  "\t%f", "\n%f", "%4f",
  "%30f", "%[0-9]", "%*f"
];

$count = 1;
foreach($array_values as $array_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($float_formats as $format) {
    // with two arguments
    var_dump( sprintf($format, $array_value) );
  }
  $count++;
};

echo "Done";
}

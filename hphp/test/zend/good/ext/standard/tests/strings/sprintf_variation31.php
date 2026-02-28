<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : octal formats with array values ***\n";

// different arrays used to test the function 
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
  vec[0123],
  dict[1 => "One", "two" => 2]
];

// array of octal formats
$octal_formats = vec[ 
  "%o", "%ho", "%lo", 
  "%Lo", " %o", "%o ",                        
  "\t%o", "\n%o", "%4o",
  "%30o", "%[0-7]", "%*o"
];

$count = 1;
foreach($array_values as $array_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($octal_formats as $format) {
    var_dump( sprintf($format, $array_value) );
  }
  $count++;
};

echo "Done";
}

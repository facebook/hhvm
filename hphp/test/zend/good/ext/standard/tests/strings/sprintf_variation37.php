<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : hexa formats with array values ***\n";

// array of array values 
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
  dict[1 => "One", "two" => 2]
];

// array of hexa formats
$hexa_formats = vec[  
  "%x", "%xx", "%lx", 
  "%Lx", " %x", "%x ",
  "\t%x", "\n%x", "%4x",
  "%30x", "%[0-9A-Fa-f]", "%*x"
];

$count = 1;
foreach($array_values as $array_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($hexa_formats as $format) {
    var_dump( sprintf($format, $array_value) );
  }
  $count++;
};

echo "Done";
}

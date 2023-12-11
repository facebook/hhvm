<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : unsigned formats with array values ***\n";

// array of array values 
$array_values = vec[
  vec[],
  vec[0],
  vec[1],
  vec[-12345],
  vec[+12345],
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

// array of unsigned formats
$unsigned_formats = vec[ 
  "%u", "%hu", "%lu",
  "%Lu", " %u", "%u ",   
  "\t%u", "\n%u", "%4u", 
   "%30u", "%[0-9]", "%*u"
];

$count = 1;
foreach($array_values as $array_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($unsigned_formats as $format) {
    var_dump( sprintf($format, $array_value) );
  }
  $count++;
};

echo "Done";
}

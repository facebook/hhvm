<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : char formats with array values ***\n";

// array of array values 
$array_values = varray[
  varray[],
  varray[0],
  varray[1],
  varray[NULL],
  varray[null],
  varray["string"],
  varray[true],
  varray[TRUE],
  varray[false],
  varray[FALSE],
  varray[1,2,3,4],
  darray[1 => "One", "two" => 2]
];

// array of char formats
$char_formats = varray[ 
  "%c", "%hc", "%lc", 
  "%Lc", " %c", "%c ",
  "\t%c", "\n%c", "%4c",
  "%30c", "%[a-bA-B@#$&]", "%*c"
];

$count = 1;
foreach($array_values as $array_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($char_formats as $format) {
    var_dump( sprintf($format, $array_value) );
  }
  $count++;
};

echo "Done";
}

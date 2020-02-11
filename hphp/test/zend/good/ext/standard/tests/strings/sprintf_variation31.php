<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : octal formats with array values ***\n";

// different arrays used to test the function 
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
  varray[0123],
  darray[1 => "One", "two" => 2]
];

// array of octal formats
$octal_formats = varray[ 
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

<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : scientific formats with array values ***\n";

// array of array values 
$array_values = varray[
  varray[],
  varray[0],
  varray[1],
  varray[100000000000],
  varray[.0000001],
  varray[10e2],
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

// array of scientific formats
$scientific_formats = varray[ 
  "%e", "%he", "%le",
  "%Le", " %e", "%e ",
  "\t%e", "\n%e", "%4e", 
  "%30e", "%[0-1]", "%*e"
];

$count = 1;
foreach($array_values as $array_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($scientific_formats as $format) {
    var_dump( sprintf($format, $array_value) );
  }
  $count++;
};

echo "Done";
}

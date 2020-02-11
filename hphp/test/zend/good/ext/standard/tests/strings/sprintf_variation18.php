<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL & ~E_NOTICE);

echo "*** Testing sprintf() : string formats with array values ***\n";

// different arrays used to test he function
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

// array of string formats
$string_formats = varray[ 
  "%s", "%hs", "%ls", 
  "%Ls"," %s", "%s ", 
  "\t%s", "\n%s", "%4s",
  "%30s", "%[a-zA-Z0-9]", "%*s"
];

$count = 1;
foreach($array_values as $array_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($string_formats as $format) {
    var_dump( sprintf($format, $array_value) );
  }
  $count++;
};

echo "Done";
}

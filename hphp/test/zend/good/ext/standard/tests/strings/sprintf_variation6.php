<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : integer formats with arrays ***\n";

// different arrays used to test the function
$array_types = varray [
  varray[],
  varray[0],
  varray[1],
  varray[-123],
  varray["123"],
  varray["-123"],
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

// various integer formats
$int_formats = varray[
  "%d", "%hd", "%ld",
  "%Ld", " %d", "%d ",
  "\t%d", "\n%d", "%4d", 
  "%30d", "%[0-9]", "%*d"
];
 
$count = 1;
foreach($array_types as $arr) {
  echo "\n-- Iteration $count --\n";
  
  foreach($int_formats as $format) {
    var_dump( sprintf($format, $arr) );
  }
  $count++;
};

echo "Done";
}

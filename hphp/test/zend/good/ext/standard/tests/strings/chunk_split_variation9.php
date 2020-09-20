<?hh
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* passing different double quoted strings for 'ending' argument to chunk_split()
* here 'chunklen' is set to 6.5
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing chunk_split() : different strings for 'ending' ***\n";

//Initializing variables
$str = "This is to test chunk_split() with various ending string";
$chunklen = 6;

//different values for 'ending' argument
$values = varray [
  "",  //empty
  " ",  //space
  "a",  //Single char
  "ENDING",  //regular string
  "@#$%^",  //Special chars

  // white space chars
  "\t",
  "\n",
  "\r",
  "\r\n",

  "\0",  //Null char
  "123",  //Numeric
  "(MSG)",  //With ( and )
  ")ending string(",  //sentence as ending string
  ")numbers 1234(",
  ")speci@! ch@r$("
];

//loop through element of values for 'ending'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration $count --\n";
  try { var_dump( chunk_split($str, $chunklen, $values[$count]) ); } catch (Exception $e) { var_dump($e->getMessage()); }
}

echo "Done";
}

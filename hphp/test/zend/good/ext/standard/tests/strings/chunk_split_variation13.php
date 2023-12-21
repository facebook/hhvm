<?hh
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions:
*/

/*
* passing long string as 'str' and testing default value of chunklen which is 76
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing chunk_split() : default 'chunklen' with long string 'str' ***\n";

//Initializing variables
$values = vec[
  "123456789012345678901234567890123456789012345678901234567890123456789012345678901",
  "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901"
];

//loop through each element of values for 'str' and default value of 'chunklen'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration $count --\n";
  var_dump( chunk_split($values[$count]) );
}

echo "Done";
}

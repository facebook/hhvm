<?hh
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* passing different single quoted strings as 'str' argument to the function
* 'chunklen' is set to 7 and 'ending' is '):('
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing chunk_split() : with different single quoted 'str' ***\n";

//Initializing variables
$chunklen = 7;
$ending = "):(";

//different single quoted string for 'str'
$values = vec[
  '',  //empty
  ' ',  //space
  'This is simple string',  //regular string
  'It\'s string with quotes',
  'This contains @ # $ % ^ & chars',   //special characters
  'This string\tcontains\rwhite space\nchars',  //with white space chars
  'This is string with 1234 numbers',
  'This is string with \0 and ".chr(0)."null chars',  //for binary safe
  'This is string with    multiple         space char',
  'This is to check string with ()',
  '     Testing with    multiple spaces     ',
  'Testing invalid \k and \m escape char',
  'This is to check with \\n and \\t'
];


//Loop through each element of values for 'str'
for($count = 0;$count < count($values);$count++) {
  echo "-- Iteration $count --\n";
  var_dump( chunk_split($values[$count], $chunklen, $ending) );
}

echo "Done";
}

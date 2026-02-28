<?hh
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* Passing different double quoted strings for 'str' argument to chunk_split()
* here 'chunklen' is set to 5 and 'ending' is "????"
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing chunk_split() : with different double quoted values for 'str' argument ***\n";

// Initializing variables
$chunklen = 5;
$ending = "????";

// different values for 'str'
$values = vec[
  "",  //empty
  " ",  //space
  "This is simple string",  //regular string
  "It's string with quotes",  //string containing single quote
  "This contains @ # $ % ^ & chars",   //string with special characters
  "This string\tcontains\rwhite space\nchars",
  "This is string with 1234 numbers",
  "This is string with \0 and ".chr(0)."null chars",  //for binary safe
  "This is string with    multiple         space char",
  "Testing invalid \k and \m escape char",
  "This is to check with \\n and \\t" //to ignore \n and \t results

];

// loop through each element of the array for 'str'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  var_dump( chunk_split( $values[$count], $chunklen, $ending) );
}

echo "Done";
}

<?hh
/* Prototype  : array str_split(string $str [, int $split_length])
 * Description: Convert a string to an array. If split_length is
                specified, break the string down into chunks each
                split_length characters long.
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* passing different double quoted strings as 'str' argument to str_split()
* split_length is set to 7
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing str_split() : double quoted strings for 'str' ***\n";

//Initialize variables
$split_length = 7;

// different values for 'str'
$values = varray[
  "",  //empty
  " ",  //space
  "1234", //with only numbers
  "simple string",  //regular string
  "It's string with quote",  //string containing single quote
  "string\tcontains\rwhite space\nchars",
  "containing @ # $ % ^ & chars",
  "with 1234 numbers",
  "with \0 and ".chr(0)."null chars",  //for binary safe
  "with    multiple     space char",
  "Testing invalid \k and \m escape char",
  "to check with \\n and \\t" //ignoring \n and \t results

];

//loop through each element of $values for 'str' argument
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  var_dump( str_split($values[$count], $split_length) );
}
echo "Done";
}

<?php
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* passing different double quoted strings for 'ending' argument to chunk_split()
* here 'chunklen' is set to 6.5
*/

echo "*** Testing chunk_split() : different strings for 'ending' ***\n";

//Initializing variables
$str = "This is to test chunk_split() with various ending string";
$chunklen = 6.5;

//different values for 'ending' argument
$values = array (
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
);

//loop through element of values for 'ending'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration $count --\n";
  var_dump( chunk_split($str, $chunklen, $values[$count]) );
}

echo "Done"
?>
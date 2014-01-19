<?php
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* passing different single quoted strings for 'ending' arguments to chunk_split()
* 'chunklen' is set to 9.2 for this testcase
*/

echo "*** Testing chunk_split() : different single quoted strings as 'ending' ***\n";


//Initializing variables
$str = "This is to test chunk_split() with various 'single quoted' ending string.";
$chunklen = 9.2;

//different values for 'ending' argument
$values = array (
  '',  //empty
  ' ',  //space
  'a',  //Single char
  'ENDING',  //String
  '@#$%^',  //Special chars

  
  '\t',  
  '\n',
  '\r',
  '\r\n',
  
  '\0',  //Null char
  '123',  //Numeric
  '(MSG)',  //With ( and )
  ') ending string (',  //sentence as ending string
  ') numbers 1234 (',  //string with numbers
  ') speci@! ch@r$ ('  //string with special chars
);


//loop through each element of values for 'ending'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration $count --\n";
  var_dump( chunk_split($str, $chunklen, $values[$count]) );
}

echo "Done"
?>
<?php
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* passsing different integer values for 'chunklen' argument to chunk_split()
* 'ending' is set to '||'
*/

echo "*** Testing chunk_split() : different integer values for 'chunklen' ***\n";

 //Initializing variables
$ending = "||";
$str = "This contains\tand special char & numbers 123.\nIt also checks for \0 char";

// different values for chunklen
$values = array (
  0,  
  1,  
  -123,  //negative integer
  0234,  //octal number
  0x1A,  //hexadecimal number
  PHP_INT_MAX,  //max positive integer number
  PHP_INT_MAX * 3,  //integer overflow
  -PHP_INT_MAX - 1,  //min negative integer

);

for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration $count --\n";
  var_dump( chunk_split($str, $values[$count], $ending) );
}

echo "Done"
?>
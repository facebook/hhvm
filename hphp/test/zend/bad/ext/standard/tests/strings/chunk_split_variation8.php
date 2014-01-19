<?php
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: 
*/

/*
* passing different integer values for 'chunklen' and heredoc string for 'str' to chunk_split()
*/

echo "*** Testing chunk_split() : different 'chunklen' with heredoc 'str' ***\n";


// Initializing required variables
//heredoc string as str
$heredoc_str = <<<EOT
This's heredoc string with \t and \n white space char.
It has _speci@l ch@r$ 2222 !!!Now \k as escape char to test
chunk_split()
EOT;

$ending = ':::';

// different values for 'chunklen'
$values = array (
  0,  
  1,  
  -123,  //negative integer
  0234,  //octal number
  0x1A,  //hexadecimal number
  PHP_INT_MAX,      // max positive integer number
  PHP_INT_MAX * 3,  // integer overflow
  -PHP_INT_MAX - 1, // min negative integer

);


// loop through each element of values for 'chunklen'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1). " --\n";
  var_dump( chunk_split($heredoc_str, $values[$count], $ending) );
}

echo "Done"
?>
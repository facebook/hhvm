<?php
/* Prototype  : string crc32(string $str)
 * Description: Calculate the crc32 polynomial of a string 
 * Source code: ext/standard/crc32.c
 * Alias to functions: none
*/

/*
* Testing crc32() : with different strings in single quotes passed to the function
*/

echo "*** Testing crc32() : with different strings in single quotes ***\n";

// defining an array of strings
$string_array = array(
  '',
  ' ',
  'hello world',
  'HELLO WORLD',
  ' helloworld ',

  '(hello world)',
  'hello(world)',
  'helloworld()',
  'hello()(world',

  '"hello" world',
  'hello "world"',
  'hello""world',

  'hello\tworld',
  'hellowor\\tld',
  '\thello world\t',
  'hello\nworld',
  'hellowor\\nld',
  '\nhello world\n',
  '\n\thelloworld',
  'hel\tlo\n world',

  '!@#$%&',
  '#hello@world.com',
  '$hello$world',

  'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbb
   cccccccccccccccccccccccccccccccccddddddddddddddddddddddddddddddddd
   eeeeeeeeeeeeeeeeeeeeeeeeeeeeffffffffffffffffffffffffffffffffffffff
   gggggggggggggggggggggggggggggggggggggggggghhhhhhhhhhhhhhhhhhhhhhhh
   111111111111111111111122222222222222222222222222222222222222222222
   333333333333333333333333333333333334444444444444444444444444444444
   555555555555555555555555555555555555555555556666666666666666666666
   777777777777777777777777777777777777777777777777777777777777777777
   /t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t
   /n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n'
);

// looping to check the behaviour of the function for each string in the array

$count = 1; 
foreach($string_array as $str) {
  echo "\n-- Iteration $count --\n";
  var_dump( crc32($str) );
  $count++;
}

echo "Done";
?>
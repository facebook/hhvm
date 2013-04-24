<?php
/* Prototype  : array array_flip(array $input)
 * Description: Return array with key <-> value flipped 
 * Source code: ext/standard/array.c
*/

/*
* Trying different keys in $input array argument
*/

echo "*** Testing array_flip() : different keys for 'input' array argument ***\n";

// different heredoc strings
$empty_heredoc = <<<EOT1
EOT1;
  
$simple_heredoc = <<<EOT4
simple
EOT4;

$multiline_heredoc = <<<EOT7
multiline heredoc with 123 and
speci@! ch@r$...checking\nand\talso
EOT7;
  
$input = array(
  // default key
  'one',  //expected: default key 0, value will be replaced by 'bool_key4'  

  // numeric keys
  1 => 'int_key', // expected: value will be replaced by 'bool_key3'
  -2 => 'negative_key',
  8.9 => 'float_key', 
  012 => 'octal_key',
  0x34 => 'hex_key',

  // string keys
  'key' => 'string_key1',
  "two" => 'string_key2',
  '' => 'string_key3',
  "" => 'string_key4',
  " " => 'string_key5',
  
  // bool keys
  true => 'bool_key1',
  false => 'bool_key2',
  TRUE => 'bool_key3', 
  FALSE => 'bool_key4',

  // null keys
  null => 'null_key1',  // expected: value will be replaced by 'null_key2'
  NULL => 'null_key2',

  // binary key
  "a".chr(0)."b" => 'binary_key1',
  b"binary" => 'binary_key2',

  //heredoc keys
  $empty_heredoc => 'empty_heredoc',
  $simple_heredoc => 'simple_heredoc',
  $multiline_heredoc => 'multiline_heredoc',
);
   
var_dump( array_flip($input) );

echo "Done"
?>
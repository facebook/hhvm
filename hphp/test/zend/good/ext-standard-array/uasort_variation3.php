<?php
/* Prototype  : bool uasort(array $array_arg, string $cmp_function)
 * Description: Sort an array with a user-defined comparison function and maintain index association 
 * Source code: ext/standard/array.c
*/

/*
* Testing uasort() with 'array_arg' having different keys
*/

echo "*** Testing uasort() : Sorting array with all possible keys ***\n";

//comparison function
/* Prototype : int cmp_function(mixed $value1, mixed $value2)
 * Parameters : $value1 and $value2 - values to be compared
 * Return value : 0 - if both values are same
 *                1 - if value1 is greater than value2
 *               -1 - if value1 is less than value2
 * Description : compares value1 and value2
 */
function cmp_function($value1, $value2)
{
  if($value1 == $value2) {
    return 0;
  }
  else if($value1 > $value2) {
    return -1;
  }
  else {
    return 1;
  }
}

// different heredoc strings
//empty heredoc string
$empty_heredoc = <<<EOT1
EOT1;
  
// single line heredoc string
$simple_heredoc = <<<EOT2
simple
EOT2;
  
// multiline heredoc string
$multiline_heredoc = <<<EOT3
multiline heredoc with 123
and speci@! ch@r..\ncheck\talso
EOT3;
  
$array_arg = array(
  // default key
  1,  //expecting: default key 0, value will be replaced by 'FALSE'  

  // numeric keys
  1 => 10, // expecting: value will be replaced by 'TRUE'
  -2 => 9,
  8.9 => 8, 
  012 => 7,
  0x34 => 6,

  // string keys
  'key' => 5,  //single quoted key
  "two" => 4,  //double quoted key
  '' => 3,
  "" => 2,  
  " " => 0,  // space as key
  
  // bool keys
  true => 15,
  false => 5,
  TRUE => 100, 
  FALSE => 25,

  // null keys
  null => 20,  // expecting: value will be replaced by 'NULL'
  NULL => 35,

  // binary key
  "a".chr(0)."b" => 45,
  b"binary" => 30,

  //heredoc keys
  $empty_heredoc => 90,
  $simple_heredoc => 75,
  $multiline_heredoc => 200,
);

var_dump( uasort($array_arg, 'cmp_function') );
echo "-- Sorted array after uasort() function call --\n";
var_dump($array_arg);

echo "Done"
?>
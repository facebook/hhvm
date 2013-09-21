<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : integer formats with string values ***\n";

// array of strings used to test the function
$string_values = array (
  "",
  '',
  "0",
  '0',
  "1",
  '1',
  "-123abc",
  "123abc",
  "+123abc",
  "\x01",
  '\x01',
  "\01",
  '\01',
  'string',
  "string",
  "true",
  "FALSE",
  'false',
  'TRUE',
  "NULL",
  'null'
);

// various integer formats
$int_formats = array(
  "%d", "%hd", "%ld",
  "%Ld", " %d", "%d ",
  "\t%d", "\n%d", "%4d",
  "%30d", "%[0-9]", "%*d"
);
 
$count = 1;
foreach($string_values as $string_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($int_formats as $format) {
    var_dump( sprintf($format, $string_value) );
  }
  $count++;
};

echo "Done";
?>
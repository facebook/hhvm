<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : unsigned formats with string values ***\n";

// array of string values 
$string_values = array(
  "",
  '',
  "0",
  '0',
  "1",
  '1',
  "-123",
  "+123",
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

// array of unsigned formats
$unsigned_formats = array( 
  "%u", "%hu", "%lu",
  "%Lu", " %u", "%u ",   
  "\t%u", "\n%u", "%4u", 
   "%30u", "%[0-9]", "%*u"
);


$count = 1;
foreach($string_values as $string_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($unsigned_formats as $format) {
    var_dump( sprintf($format, $string_value) );
  }
  $count++;
};

echo "Done";
?>
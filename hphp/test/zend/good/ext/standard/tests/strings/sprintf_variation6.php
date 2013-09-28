<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : integer formats with arrays ***\n";

// different arrays used to test the function
$array_types = array (
  array(),
  array(0),
  array(1),
  array(-123),
  array("123"),
  array("-123"),
  array(NULL),
  array(null),
  array("string"),
  array(true),
  array(TRUE),
  array(false),
  array(FALSE),
  array(1,2,3,4),
  array(1 => "One", "two" => 2)
);

// various integer formats
$int_formats = array(
  "%d", "%hd", "%ld",
  "%Ld", " %d", "%d ",
  "\t%d", "\n%d", "%4d", 
  "%30d", "%[0-9]", "%*d"
);
 
$count = 1;
foreach($array_types as $arr) {
  echo "\n-- Iteration $count --\n";
  
  foreach($int_formats as $format) {
    var_dump( sprintf($format, $arr) );
  }
  $count++;
};

echo "Done";
?>
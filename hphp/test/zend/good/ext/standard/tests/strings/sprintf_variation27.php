<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/


echo "*** Testing sprintf() : char formats with char values ***\n";

// array of char values 
$char_values = array( 'a', "a", 67, -67, 99, ' ', '', 'A', "A" );

// array of char formats
$char_formats = array( 
  "%c", "%hc", "%lc", 
  "%Lc", " %c", "%c ",
  "\t%c", "\n%c", "%4c",
  "%30c", "%[a-bA-B@#$&]", "%*c"
);

$count = 1;
foreach($char_values as $char_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($char_formats as $format) {
    var_dump( sprintf($format, $char_value) );
  }
  $count++;
};

echo "Done";
?>
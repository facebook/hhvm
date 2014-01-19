<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : string formats with boolean values ***\n";

// array of boolean values 
$boolean_values = array(
  true,
  false,
  TRUE,
  FALSE,
);

// array of string formats
$string_formats = array( 
  "%s", "%hs", "%ls", 
  "%Ls"," %s", "%s ", 
  "\t%s", "\n%s", "%4s",
  "%30s", "%[a-zA-Z0-9]", "%*s"
);

$count = 1;
foreach($boolean_values as $boolean_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($string_formats as $format) {
    var_dump( sprintf($format, $boolean_value) );
  }
  $count++;
};

echo "Done";
?>
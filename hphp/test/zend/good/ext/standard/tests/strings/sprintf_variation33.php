<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : octal formats with boolean values ***\n";

// array of boolean values 
$boolean_values = array(
  true,
  false,
  TRUE,
  FALSE,
);

// array of octal formats
$octal_formats = array( 
  "%o", "%ho", "%lo", 
  "%Lo", " %o", "%o ",                        
  "\t%o", "\n%o", "%4o",
  "%30o", "%[0-7]", "%*o"
);

$count = 1;
foreach($boolean_values as $boolean_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($octal_formats as $format) {
    var_dump( sprintf($format, $boolean_value) );
  }
  $count++;
};

echo "Done";
?>
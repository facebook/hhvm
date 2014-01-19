<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : octal formats with float values ***\n";

// array of float values 
$float_values = array(
  0.0,
  -0.1,
  1.0,
  1e5,
  -1e6,
  1E8,
  -1E9,
  10.5e+5
);

// array of octal formats
$octal_formats = array( 
  "%o", "%ho", "%lo", 
  "%Lo", " %o", "%o ",                        
  "\t%o", "\n%o", "%4o",
  "%30o", "%[0-7]", "%*o"
);

$count = 1;
foreach($float_values as $float_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($octal_formats as $format) {
    var_dump( sprintf($format, $float_value) );
  }
  $count++;
};

echo "Done";
?>
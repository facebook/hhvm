<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : scientific formats with boolean values ***\n";

// array of boolean values 
$boolean_values = varray[
  true,
  false,
  TRUE,
  FALSE,
];

// array of scientific formats
$scientific_formats = varray[ 
  "%e", "%he", "%le",
  "%Le", " %e", "%e ",
  "\t%e", "\n%e", "%4e", 
  "%30e", "%[0-1]", "%*e"
];

$count = 1;
foreach($boolean_values as $boolean_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($scientific_formats as $format) {
    var_dump( sprintf($format, $boolean_value) );
  }
  $count++;
};

echo "Done";
}

<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : unsigned formats with boolean values ***\n";

// array of boolean values 
$boolean_values = vec[
  true,
  false,
  TRUE,
  FALSE,
];

// array of unsigned formats
$unsigned_formats = vec[ 
  "%u", "%hu", "%lu",
  "%Lu", " %u", "%u ",   
  "\t%u", "\n%u", "%4u", 
   "%30u", "%[0-9]", "%*u"
];

$count = 1;
foreach($boolean_values as $boolean_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($unsigned_formats as $format) {
    var_dump( sprintf($format, $boolean_value) );
  }
  $count++;
};

echo "Done";
}

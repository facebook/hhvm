<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : char formats with resource values ***\n";

// resource type variable
$fp = fopen (__FILE__, "r");
$dfp = opendir ( dirname(__FILE__) );

// array of resource types
$resource_values = vec[
  $fp,
  $dfp
];

// array of char formats
$char_formats = vec[ 
  "%c", "%hc", "%lc", 
  "%Lc", " %c", "%c ",
  "\t%c", "\n%c", "%4c",
  "%30c", "%[a-bA-B@#$&]", "%*c"
];

$count = 1;
foreach($resource_values as $resource_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($char_formats as $format) {
    var_dump( sprintf($format, $resource_value) );
  }
  $count++;
};

// closing the resources
fclose($fp);
closedir($dfp);

echo "===DONE===\n";
}

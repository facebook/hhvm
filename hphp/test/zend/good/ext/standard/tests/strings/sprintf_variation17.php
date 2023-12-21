<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : string formats with resource values ***\n";

// resource type variable
$fp = fopen (__FILE__, "r");
$dfp = opendir ( dirname(__FILE__) );

// array of resource values
$resource_values = vec[
  $fp,
  $dfp
];

// array of string formats
$string_formats = vec[ 
  "%s", "%hs", "%ls", 
  "%Ls"," %s", "%s ", 
  "\t%s", "\n%s", "%4s",
  "%30s", "%[a-zA-Z0-9]", "%*s"
];

$count = 1;
foreach($resource_values as $resource_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($string_formats as $format) {
    var_dump( sprintf($format, $resource_value) );
  }
  $count++;
};

// closing the resources
fclose($fp);
closedir($dfp);

echo "Done";
}

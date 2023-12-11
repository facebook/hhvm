<?hh
/* Prototype  : string file_get_contents(string filename [, bool use_include_path [, resource context [, long offset [, long maxlen]]]])
 * Description: Read the entire file into a string 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing file_get_contents() : variation ***\n";
/* An array of filenames */ 
$names_arr = vec[
  /* Invalid args */ 
  -1,
  TRUE,
  FALSE,
  NULL,
  "",
  " ",
  "\0",
  vec[],

  /* prefix with path separator of a non existing directory*/
  "/no/such/file/dir", 
  "php/php"

];

for( $i=0; $i<count($names_arr); $i++ ) {
  echo "-- Iteration $i --\n";
  try { var_dump(file_get_contents($names_arr[$i])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

echo "\n*** Done ***\n";
}

<?hh
/* Prototype  : int readfile(string filename [, bool use_include_path[, resource context]])
 * Description: Output a file or a URL
 * Source code: ext/standard/file.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing readfile() : variation ***\n";


/* An array of files */
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
  $name = $names_arr[$i];
  $text = HH\is_any_array($name) ? 'Array' : $name;
  echo "-- testing '".(string)$text."' --\n";
  try { readfile($name); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

echo "\n*** Done ***\n";
}

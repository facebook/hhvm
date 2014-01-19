<?php
/* Prototype  : string file_get_contents(string filename [, bool use_include_path [, resource context [, long offset [, long maxlen]]]])
 * Description: Read the entire file into a string 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing file_get_contents() : variation ***\n";
/* An array of filenames */ 
$names_arr = array(
  /* Invalid args */ 
  -1,
  TRUE,
  FALSE,
  NULL,
  "",
  " ",
  "\0",
  array(),

  /* prefix with path separator of a non existing directory*/
  "/no/such/file/dir", 
  "php/php"

);

for( $i=0; $i<count($names_arr); $i++ ) {
  echo "-- Iteration $i --\n";
  var_dump(file_get_contents($names_arr[$i]));
}

echo "\n*** Done ***\n";
?>
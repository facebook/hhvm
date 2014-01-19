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
  "-1" => -1,
  "TRUE" => TRUE,
  "FALSE" => FALSE,
  "NULL" => NULL,
  "\"\"" => "",
  "\" \"" => " ",
  "\\0" => "\0",
  "array()" => array(),

  /* prefix with path separator of a non existing directory*/ 
  "/no/such/file/dir" => "/no/such/file/dir", 
  "php/php"=> "php/php"

);

foreach($names_arr as $key =>$value) {
  echo "\n-- Filename: $key --\n";
  var_dump(file_get_contents($value));
}

?>
===Done===
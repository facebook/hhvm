<?php
/* Prototype  : int readfile(string filename [, bool use_include_path[, resource context]])
 * Description: Output a file or a URL 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing readfile() : variation ***\n";

/* An array of files */ 
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

foreach($names_arr as $key => $value) {
      echo "\n-- Filename: $key --\n";
      readfile($value);
};

?>
===Done===
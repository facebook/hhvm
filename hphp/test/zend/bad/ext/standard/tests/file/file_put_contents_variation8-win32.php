<?php
/* Prototype  : int file_put_contents(string file, mixed data [, int flags [, resource context]])
 * Description: Write/Create a file with contents data and return the number of bytes written 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing file_put_contents() : usage variation ***\n";

/* An array of filenames */ 
$names_arr = array(
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
      $res = file_put_contents($value, "Some data");
  	  if ($res !== false && $res != null) {
     	 echo "$res bytes written to: $value\n";
     	 unlink($value);
  	  } else {
         echo "Failed to write data to: $key\n";
      }	
};

?>
===Done===
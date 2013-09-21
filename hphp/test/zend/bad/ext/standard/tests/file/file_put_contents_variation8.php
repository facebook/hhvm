<?php
/* Prototype  : int file_put_contents(string file, mixed data [, int flags [, resource context]])
 * Description: Write/Create a file with contents data and return the number of bytes written 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing file_put_contents() : usage variation ***\n";

/* An array of filenames */ 
$names_arr = array(
  -1,
  TRUE,
  FALSE,
  NULL,
  "",
  " ",
  //this one also generates a java message rather than our own so we don't replicate php message
  "\0",
  array(),

  //the next 2 generate java messages so we don't replicate the php messages
  "/no/such/file/dir", 
  "php/php"

);

for( $i=0; $i<count($names_arr); $i++ ) {
  echo "-- Iteration $i --\n";
  $res = file_put_contents($names_arr[$i], "Some data");
  if ($res !== false && $res != null) {
     echo "$res bytes written to: $names_arr[$i]\n";
     unlink($names_arr[$i]);
  }
  else {
     echo "Failed to write data to: $names_arr[$i]\n";
  }
}

echo "\n*** Done ***\n";
?>
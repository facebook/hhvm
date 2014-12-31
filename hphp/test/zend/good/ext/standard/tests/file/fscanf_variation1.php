<?php

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/* test fscanf() for its return type */

$file_path = dirname(__FILE__);

echo "*** Testing fscanf(): for its return type without third argument ***\n";

// create a file
$filename = "$file_path/fscanf_variation1.tmp";
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");
@fwrite($file_handle, "hello_world ");
@fwrite($file_handle, 12345);
fclose($file_handle);

// open file for reading 
$file_handle = fopen($filename, "r");
// capturing the return value from fscanf() called without third argument
$return_value = fscanf($file_handle, "%s");
var_dump( is_array($return_value), $return_value); // return type is an array
fclose($file_handle);

echo "\n*** Done ***";
?>
<?php error_reporting(0); ?>
<?php
$file_path = dirname(__FILE__);
$filename = "$file_path/fscanf_variation1.tmp";
unlink($filename); 
?>
<?php

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/* Test fscanf() to scan arrays using different string format types */

$file_path = dirname(__FILE__);

echo "*** Test fscanf(): different string format types with arrays ***\n"; 

// create a file
$filename = "$file_path/fscanf_variation17.tmp";
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");

// array types
$array_types = array (
  array(),
  array(0),
  array(1),
  array(NULL),
  array(null),
  array("string"),
  array(true),
  array(TRUE),
  array(false),
  array(FALSE),
  array(1,2,3,4),
  array(1 => "One", "two" => 2)
);

$string_formats = array( "%s",
                         "%hs", "%ls", "%Ls", 
                         " %s", "%s ", "% s",
                         "\t%s", "\n%s", "%4s",
                         "%30s", "%[a-zA-Z0-9]", "%*s");

$counter = 1;

// writing to the file
foreach($array_types as $value) {
  @fprintf($file_handle, $value);
  @fprintf($file_handle, "\n");
}
// closing the file
fclose($file_handle);

// opening the file for reading
$file_handle = fopen($filename, "r");
if($file_handle == false) {
  exit("Error:failed to open file $filename");
}

$counter = 1;
// reading the values from file using different string formats
foreach($string_formats as $string_format) {
  // rewind the file so that for every foreach iteration the file pointer starts from bof
  rewind($file_handle);
  echo "\n-- iteration $counter --\n";
  while( !feof($file_handle) ) {
    var_dump( fscanf($file_handle,$string_format) );
  }
  $counter++;
}

echo "\n*** Done ***";
?>
<?php error_reporting(0); ?>
<?php
$file_path = dirname(__FILE__);
$filename = "$file_path/fscanf_variation17.tmp";
unlink($filename);
?>
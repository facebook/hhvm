<?php

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/* Test fscanf() to scan resource type using different hexa format types */

$file_path = dirname(__FILE__);

echo "*** Test fscanf(): different hexa format types with resource ***\n"; 

// create a file
$filename = "$file_path/fscanf_variation35.tmp";
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");


// resource type variable
$fp = fopen (__FILE__, "r");
$dfp = opendir ( dirname(__FILE__) );
  
// array of resource types
$resource_types = array (
  $fp,
  $dfp
);

$hexa_formats = array( "%x", "%hx", "%lx", "%Lx", " %x", "%x ", "% x", "\t%x", "\n%x", "%4x", "%30x", "%[0-9]", "%*x");

$counter = 1;

// writing to the file
foreach($resource_types as $value) {
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
// reading the values from file using different hexa formats
foreach($hexa_formats as $hexa_format) {
  // rewind the file so that for every foreach iteration the file pointer starts from bof
  rewind($file_handle);
  echo "\n-- iteration $counter --\n";
  while( !feof($file_handle) ) {
    var_dump( fscanf($file_handle,$hexa_format) );
  }
  $counter++;
}

// closing the resources
fclose($fp);
closedir($dfp);

echo "\n*** Done ***";
?>
<?php error_reporting(0); ?>
<?php
$file_path = dirname(__FILE__);
$filename = "$file_path/fscanf_variation35.tmp";
unlink($filename);
?>
<?php

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/* Test fscanf() to scan arrays using different octal format types */

$file_path = dirname(__FILE__);

echo "*** Test fscanf(): different octal format types with arrays ***\n"; 

// create a file
$filename = "$file_path/fscanf_variation30.tmp";
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

$octal_formats = array ( "%o",
 		         "%ho", "%lo", "%Lo",
			 " %o", "%o ", "% o",
			 "\t%o", "\n%o", "%4o",
			 "%30o", "%[0-7]", "%*o"
		 );

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
// reading the values from file using different octal formats
foreach($octal_formats as $octal_format) {
  // rewind the file so that for every foreach iteration the file pointer starts from bof
  rewind($file_handle);
  echo "\n-- iteration $counter --\n";
  while( !feof($file_handle) ) {
    var_dump( fscanf($file_handle,$octal_format) );
  }
  $counter++;
}

echo "\n*** Done ***";
?><?php
$file_path = dirname(__FILE__);
$filename = "$file_path/fscanf_variation30.tmp";
unlink($filename);
?>
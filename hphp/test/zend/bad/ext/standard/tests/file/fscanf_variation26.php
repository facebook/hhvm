<?php

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/* Test fscanf() to scan different chars using different char format types */

$file_path = dirname(__FILE__);

echo "*** Test fscanf(): different char format types with chars ***\n"; 

// create a file
$filename = "$file_path/fscanf_variation26.tmp";
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");

// array of chars
$char_types = array( 'a', "a", 67, -67, 99 );

$char_formats = array( "%c",
		       "%hc", "%lc", "%Lc",
		       " %c", "%c ", "% c",
		       "\t%c", "\n%c", "%4c",
		       "%30c", "%[a-zA-Z@#$&0-9]", "%*c");

$counter = 1;

// writing to the file
foreach($char_types as $char) {
  @fprintf($file_handle, $char);
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
// reading the values from file using different char formats
foreach($char_formats as $char_format) {
  // rewind the file so that for every foreach iteration the file pointer starts from bof
  rewind($file_handle);
  echo "\n-- iteration $counter --\n";
  while( !feof($file_handle) ) {
    var_dump( fscanf($file_handle,$char_format) );
  }
  $counter++;
}

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
$filename = "$file_path/fscanf_variation26.tmp";
unlink($filename);
?>
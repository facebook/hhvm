<?php

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/*
  Test fscanf() to scan data using different format types and also
  tracking the file pointer movement along with reading
 */

$file_path = dirname(__FILE__);

echo "*** Test fscanf(): tracking file pointer along with reading data from file ***\n"; 

// create a file
$filename = "$file_path/fscanf_variation55.tmp";
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");

// different valid data
$valid_data = array(
  12345,            // integer value
  -12345,           
  123.45,	    // float value
  -123.45,	   
  0x123B,           // hexadecimal value
  0x12ab,
  0123,             // octal value
  -0123,            
  "abcde",          // string
  'abcde',
  10e3,             // exponential value
  10e-3           
);
// various formats
$int_formats = array( "%d", "%f", "%s", "%o", "%x", "%u", "%c", "%e");

$counter = 1;

// writing to the file
foreach($valid_data as $data) {
  @fprintf($file_handle, $data);
  @fprintf($file_handle, "\n");
}
// closing the file
fclose($file_handle);

$modes = array("r", "rb", "rt");

foreach($modes as $mode) {

  echo "\n*** File opened in $mode mode ***\n";
  // opening the file for reading
  $file_handle = fopen($filename, $mode);
  if($file_handle == false) {
    exit("Error:failed to open file $filename");
  }

  $counter = 1;
  // reading different data from file using different formats
  foreach($int_formats as $int_format) {
    // current file pointer position
    var_dump( ftell($file_handle) ); 
    // rewind the file so that for every foreach iteration the file pointer starts from bof
    rewind($file_handle);
    // current file pointer position after rewind operation
    var_dump( ftell($file_handle) ); 
    echo "\n-- iteration $counter --\n";
    while( !feof($file_handle) ) {
      var_dump( ftell($file_handle) ); 
      var_dump( fscanf($file_handle,$int_format) );
    }
    $counter++;
  } // end of inner for loop
} // end of outer for loop

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
$filename = "$file_path/fscanf_variation55.tmp";
unlink($filename);
?>
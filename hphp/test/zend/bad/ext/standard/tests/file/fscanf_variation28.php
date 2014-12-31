<?php

/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

/* Test fscanf() to scan float values using different octal format types */

$file_path = dirname(__FILE__);

echo "*** Test fscanf(): different octal format types with float values ***\n"; 

// create a file
$filename = "$file_path/fscanf_variation28.tmp";
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");

// array of float type values

$float_values = array (
  -2147483649, 
  2147483648,  
  -0x80000001, // float value, beyond max negative int
  0x800000001, // float value, beyond max positive int
  020000000001, // float value, beyond max positive int
  -020000000001, // float value, beyond max negative int
  0.0,
  -0.1,
  1.0,
  1e5,
  -1e6,
  1E8,
  -1E9,
  10.0000000000000000005,
  10.5e+5
);

$octal_formats = array( "%o",
                        "%ho", "%lo", "%Lo", 
                        " %o", "%o ", "% o",
                        "\t%o", "\n%o", "%4o",
                        "%30o", "%[0-7]", "%*o"
                 );

$counter = 1;

// writing to the file
foreach($float_values as $value) {
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
?>
<?php error_reporting(0); ?>
<?php
$file_path = dirname(__FILE__);
$filename = "$file_path/fscanf_variation28.tmp";
unlink($filename);
?>
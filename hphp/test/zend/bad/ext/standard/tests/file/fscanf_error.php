<?php
/*
  Prototype: mixed fscanf ( resource $handle, string $format [, mixed &$...] );
  Description: Parses input from a file according to a format
*/

echo "*** Testing fscanf() for error conditions ***\n";
$file_path = dirname(__FILE__);

$filename = "$file_path/fscanf_error.tmp";
$file_handle = fopen($filename, 'w');
if ($file_handle == false) 
  exit("Error:failed to open file $filename");
fwrite($file_handle, "hello world");
fclose($file_handle);

// zero argument
var_dump( fscanf() );

// single argument
$file_handle = fopen($filename, 'r');
if ($file_handle == false) 
  exit("Error:failed to open file $filename");
var_dump( fscanf($file_handle) );
fclose($file_handle);

// invalid file handle
var_dump( fscanf($file_handle, "%s") );

// number of formats in format strings not matching the no of variables
$file_handle = fopen($filename, 'r');
if ($file_handle == false) 
  exit("Error:failed to open file $filename");
var_dump( fscanf($file_handle, "%d%s%f", $int_var, $string_var) );
fclose($file_handle);

// different invalid format strings
$invalid_formats = array( $undefined_var, undefined_constant,
                          "%", "%h", "%.", "%d%m"
                   );


// looping to use various invalid formats with fscanf()
foreach($invalid_formats as $format)  {
  $file_handle = fopen($filename, 'r');
  if ($file_handle == false) 
    exit("Error:failed to open file $filename");
  var_dump( fscanf($file_handle, $format) );
  fclose($file_handle); 
}

echo "\n*** Done ***";
?>
<?php
$file_path = dirname(__FILE__);
$filename = "$file_path/fscanf_error.tmp";
unlink($filename);
?>
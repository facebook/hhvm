<?php
/*  Prototype: bool fflush ( resource $handle );
    Description: Flushes the output to a file
*/

/* test fflush() with handle to the files opened in different modes */

$file_path = dirname(__FILE__);
require $file_path.'/file.inc';

echo "*** Testing fflush(): with various types of files ***\n";
$file_types = array("empty", "numeric", "text", "text_with_new_line", "alphanumeric");
$file_modes = array("w", "wb", "wt", "w+", "w+b", "w+t",
                    "a", "ab", "at", "a+","a+b", "a+t",
                    "x", "xb", "xt", "x+", "x+b", "x+t");

$file_name = "$file_path/fflush_variation1.tmp"; 

$count = 1;

foreach( $file_types as $type ) {
  echo "-- Iteration $count with file containing $type Data--\n";
  foreach( $file_modes as $mode ) {
    echo "-- File opened in $mode mode --\n";
    
    // creating the file except for x mode
    if( substr($mode, 0, 1) != "x" ) {
      $file_handle = fopen($file_name, "w");
      if($file_handle == false)
        exit("Error:failed to open file $file_name");
      
      // filling the file some data if mode is append mode
      if( substr($mode, 0, 1) == "a") 
        fill_file($file_handle, $type, 10);
      fclose($file_handle);
    } 
  
    // opening the file in different modes 
    $file_handle = fopen($file_name, $mode);
    if($file_handle == false) 
      exit("Error:failed to open file $file_name");
    
    // writing data to the file
    var_dump( fill_file($file_handle, $type, 50) ); 
    var_dump( fflush($file_handle) );
    fclose($file_handle);

    // reading the contents of the file after flushing
     var_dump( readfile($file_name) );
     unlink($file_name);
  }
  $count++;
}

echo "\n*** Done ***";
?>
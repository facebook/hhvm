<?php
/*  Prototype: bool fflush ( resource $handle );
    Description: Flushes the output to a file
*/

/* test fflush() with handle to hard links as resource */

$file_path = dirname(__FILE__);
require $file_path.'/file.inc';

echo "*** Testing fflush(): with hard links to files opened in diff modes ***\n";
$file_types = array("empty", "numeric", "text", "text_with_new_line", "alphanumeric");
$file_modes = array("w", "wb", "wt", "w+", "w+b","w+t",
                    "a", "ab", "at", "a+","a+b", "a+t");

$file_name = "$file_path/fflush_variation3.tmp";
$link_name = "$file_path/lnk_fflush_variation3.tmp";

$count = 1;

foreach( $file_types as $type ) {
  echo "-- Iteration $count with file containing $type data --\n";
  foreach( $file_modes as $mode ) {
    
    // creating the file
    $file_handle = fopen($file_name, "w");
    if($file_handle == false)
      exit("Error:failed to open file $file_name");

    // fill the fill with some data if mode is append mode 
    if( substr($mode, 0, 1) == "a" ) 
      fill_file($file_handle, $type, 10); 

    // fclose($file_handle);
   
    // creating hard link to the file
    var_dump( link($file_name, $link_name) );
  
    // opening the file in different modes
    $file_handle = fopen($link_name, $mode);
    if($file_handle == false)
      exit("Error:failed to open link $link_name");
  
    // writing data to the file
    var_dump( fill_file($file_handle, $type, 50) ); 
    var_dump( fflush($file_handle) );
    fclose($file_handle);

    // reading data from the file after flushing
    var_dump( readfile($link_name) );

    unlink($link_name);
    unlink($file_name);
  }
  $count++;
}

echo "\n*** Done ***";
?>
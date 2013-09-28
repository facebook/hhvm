<?php
/*  Prototype: bool fflush ( resource $handle );
    Description: Flushes the output to a file
*/

/* test fflush() with handle to symbollic link */

$file_path = dirname(__FILE__);
require $file_path.'/file.inc';

echo "*** Testing fflush(): with soft links to files opened in diff modes ***\n";
$file_types = array("empty", "numeric", "text", "text_with_new_line", "alphanumeric");
$file_modes = array("w", "wb", "wt", "w+", "w+b", "w+t",
                    "a", "ab", "at", "a+","a+b", "a+t");

$file_name = "$file_path/fflush_variation2.tmp";
$symlink_name = "$file_path/symlnk_fflush_variation2.tmp";

$count = 1;

foreach( $file_types as $type ) {
  echo "-- Iteration $count with file containing $type data --\n";
  foreach( $file_modes as $mode ) {
    echo "-- link opened in $mode mode --\n";

    //creating the file
    $file_handle = fopen($file_name, "w");
    if($file_handle == false)
      exit("Error:failed to open file $file_name");
  
    //fill the file with some data if mode is append mode
    if( substr($mode, 0, 1) == "a" ) 
      fill_file($file_handle, $type, 10); 
  
    //close the file
    fclose($file_handle);
  
    // creating the sym link
    var_dump( symlink($file_name, $symlink_name) );
    $file_handle = fopen($symlink_name, $mode);
    if($file_handle == false)
      exit("Error:failed to open link $symlink_name");

    // filling data into the file
    var_dump( fill_file($file_handle, $type, 50) ); 
    var_dump( fflush($file_handle) );
    fclose($file_handle);
    
    // reading the data from the file
    var_dump( readfile($symlink_name) );

    unlink($symlink_name);
    unlink($file_name);
  }
  $count++;
}

echo "\n*** Done ***";
?>
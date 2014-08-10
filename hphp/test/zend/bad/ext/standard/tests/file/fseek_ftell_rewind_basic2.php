<?php
/* Prototype: int fseek ( resource $handle, int $offset [, int $whence] );
   Description: Seeks on a file pointer

   Prototype: bool rewind ( resource $handle );
   Description: Rewind the position of a file pointer

   Prototype: int ftell ( resource $handle );
   Description: Tells file pointer read/write position
*/

// include the file.inc for common functions for test
include ("file.inc");

/* Testing fseek(),ftell(),rewind() functions on all write and create with write modes */

echo "*** Testing fseek(), ftell(), rewind() : basic operations ***\n";
$file_modes = array( "w","wb","wt","w+","w+b","w+t", 
                     "x","xb","xt","x+","x+b","x+t");

$file_content_types = array("text_with_new_line","alphanumeric");

$whence_set = array(SEEK_SET,SEEK_CUR,SEEK_END);
$whence_string = array("SEEK_SET", "SEEK_CUR", "SEEK_END");

$filename = dirname(__FILE__)."/fseek_ftell_rewind_basic2.tmp"; // this is name of the file created by create_files()

foreach($file_content_types as $file_content_type){
  echo "\n-- File having data of type ". $file_content_type ." --\n";

  /* open the file using $files_modes and perform fseek(),ftell() and rewind() on it */
  foreach($file_modes as $file_mode) {
    echo "-- File opened in mode ".$file_mode." --\n";

    $file_handle = fopen($filename, $file_mode);
    if (!$file_handle) {
      echo "Error: failed to fopen() file: $filename!";
      exit();
    }
    $data_to_be_written="";
    fill_buffer($data_to_be_written, $file_content_type, 512); //get the data of size 512
    $data_to_be_written = $data_to_be_written;
    fwrite($file_handle,(binary)$data_to_be_written);

    // set file pointer to 0
    var_dump( rewind($file_handle) ); // set to beginning of file 
    var_dump( ftell($file_handle) );

    foreach($whence_set as $whence){
      echo "-- Testing fseek() with whence = $whence_string[$whence] --\n";
      var_dump( fseek($file_handle, 10, $whence) ); //expecting int(0)
      var_dump( ftell($file_handle) ); // confirm the file pointer position
      var_dump( feof($file_handle) ); //ensure that file pointer is not at end
    } //end of whence loop

    //close the file and check the size
    fclose($file_handle);
    var_dump( filesize($filename) );

    delete_file($filename); // delete file with name
  } //end of file_mode loop
} //end of File content type loop
echo "Done\n";
?>

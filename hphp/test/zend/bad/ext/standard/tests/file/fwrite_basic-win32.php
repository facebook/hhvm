<?php
/*
 Prototype: int fwrite ( resource $handle,string string, [, int $length] );
 Description: fwrite() writes the contents of string to the file stream pointed to by handle.
              If the length arquement is given,writing will stop after length bytes have been
              written or the end of string reached, whichever comes first.
              fwrite() returns the number of bytes written or FALSE on error
*/

// include the file.inc for Function: function delete_file($filename)
include ("file.inc");

echo "*** Testing fwrite() basic operations ***\n";
/*
 test fwrite with file opened in mode : w,wb,wt,w+,w+b,w+t
 File containing data of type,  numeric, text, text_with_new_line, alphanumeric
*/
$file_modes = array( "w", "wb", "wt", "w+", "w+b", "w+t");
$file_content_types = array("numeric","text","text_with_new_line","alphanumeric");

foreach($file_content_types as $file_content_type) {
  echo "\n-- Testing fwrite() with file having data of type ". $file_content_type ." --\n";
  $filename = dirname(__FILE__)."/fwrite_basic-win32.tmp"; // this is name of the file

  for($inner_loop_counter = 0; 
      $inner_loop_counter < count($file_modes); 
      $inner_loop_counter++) {
     echo "--  File opened in mode : " . $file_modes[$inner_loop_counter]. " --\n";
     /* open the file using $files_modes and perform fwrite() on it */
     $file_handle = fopen($filename, $file_modes[$inner_loop_counter]);
     if (!$file_handle) {
       echo "Error: failed to fopen() file: $filename!";
       exit();
     }
     $data_to_be_written="";
     fill_buffer($data_to_be_written, $file_content_type, 1024);  //get the data of size 1024

    /* Write the data in to the file, verify the write by checking file pointer position, 
       eof position, and data. */
    // writing 100 bytes
    var_dump( ftell($file_handle) );  // Expecting 0
    var_dump( fwrite($file_handle, $data_to_be_written, 100)); //int(100)
    var_dump( feof($file_handle) );  // expected : false
    var_dump( ftell($file_handle) );  //expected: 100
   
    // trying to write more than the available data, available 1024 bytes but trying 2048
    var_dump( fwrite($file_handle, $data_to_be_written, 2048)); //int(1024)
    var_dump( feof($file_handle) );  // expected : false
    var_dump( ftell($file_handle) );  // expected: 1124

    // fwrite() without length parameter
    var_dump( fwrite($file_handle, $data_to_be_written)); //int(1024)
    var_dump( ftell($file_handle) );  // expected: 2148
    var_dump( feof($file_handle) );  // expected: false

    // close the file, get the size and content of the file.
    var_dump( fclose($file_handle) ); //expected : true
    clearstatcache();//clears file status cache
    var_dump( filesize($filename) );  // expected:  2148
    var_dump(md5(file_get_contents($filename))); // hash the output 

  } // end of inner for loop

  // delete the file created : fwrite_basic.tmp
  delete_file($filename);
} // end of outer foreach loop
echo "Done\n";
?>
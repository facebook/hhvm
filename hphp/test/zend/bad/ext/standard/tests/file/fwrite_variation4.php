<?php
/*
 Prototype: int fwrite ( resource $handle,string string, [, int $length] );
 Description: fwrite() writes the contents of string to the file stream pointed to by handle.
              If the length arquement is given,writing will stop after length bytes have been
              written or the end of string reached, whichever comes first.
              fwrite() returns the number of bytes written or FALSE on error
*/


echo "*** Testing fwrite() various  operations ***\n";

// include the file.inc for Function: function delete_file($filename)
include ("file.inc");

/*
 Test fwrite with file opened in mode : x, xb, xt, x+, x+b, x+t
 File having content of type numeric, text,text_with_new_line & alphanumeric
*/

$file_modes = array("x","xb","xt","x+","x+b","x+t");
$file_content_types = array("numeric","text","text_with_new_line","alphanumeric");


foreach($file_content_types as $file_content_type) {
  echo "\n-- Testing fwrite() with file having content of type ". $file_content_type ." --\n";

  /* open the file using $files_modes and perform fwrite() on it */
  foreach($file_modes as $file_mode) {
    echo "-- Opening file in $file_mode --\n";

    $filename = dirname(__FILE__)."/fwrite_variation4.tmp"; // this is name of the file

    $file_handle = fopen($filename, $file_mode);
    if(!$file_handle) {
      echo "Error: failed to fopen() file: $filename!";
      exit();
    }

    $data_to_be_written="";
    fill_buffer($data_to_be_written,$file_content_type,1024);  //get the data of size 1024

    /*  Write the data into the file, verify it by checking the file pointer position, eof position, 
        filesize & by displaying the content */
    // write data to the file
    var_dump( ftell($file_handle) );
    var_dump( fwrite($file_handle,$data_to_be_written,400));
    var_dump( ftell($file_handle) );
    var_dump( feof($file_handle) );  // expected: true

    //check the filesize and content
    // close the file, get the size and content of the file.
    var_dump( fclose($file_handle) );
    clearstatcache();//clears file status cache
    var_dump( filesize($filename) );
    var_dump(md5(file_get_contents($filename)));
    // delete the file created
    delete_file($filename); // delete file with name fwrite_variation4.tmp
  } // end of inner foreach loop
} // end of outer foreach loop

echo "Done\n";
?>
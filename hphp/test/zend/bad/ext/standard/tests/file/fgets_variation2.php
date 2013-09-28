<?php
/*
 Prototype: string fgets ( resource $handle [, int $length] );
 Description: Gets a line from file pointer
*/

/* try reading a line using fgets() using invalid handles
    - closed file handle
    - unset file handle
*/
 
// include the header for common test function 
include ("file.inc");

echo "*** Testing fgets() : usage variations ***\n";

echo "-- Testing fgets() with closed handle --\n";
// open the file for reading
$file_handle = fopen(__FILE__, "r");
// close the file
fclose($file_handle);

// read from closed file
var_dump( fgets($file_handle) ); // default length
var_dump( fgets($file_handle, 10) ); // with specific length

echo "-- Testing fgets() with unset handle --\n";
// open the file for reading
$file_handle = fopen(__FILE__, "r");
// unset the file handle
unset($file_handle);

//fgets using unset handle
var_dump( fgets($file_handle) ); // default length
var_dump( fgets($file_handle, 10) ); // with specific length

echo "Done";
?>
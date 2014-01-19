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

echo "*** Testing fwrite() : error conditions ***\n";

$filename = dirname(__FILE__)."/fwrite_error.tmp";

echo "-- Testing fwrite() with less than expected number of arguments --\n";
// zero argument
var_dump( fwrite() ); 
// less than expected, 1 arg
$file_handle  = fopen ( $filename, "w");
var_dump( fwrite($file_handle) );

// more than expected no. of args
echo "-- Testing fwrite() with more than expected number of arguments --\n";
$data = "data";
var_dump( fwrite($file_handle, $data, strlen($data), 10) );

// invalid length argument
echo "-- Testing fwrite() with invalid length arguments --\n";
$len = 0;
var_dump( fwrite($file_handle, $data, $len) );
$len = -10;
var_dump( fwrite($file_handle, $data, $len) );

// test invalid arguments : non-resources
echo "-- Testing fwrite() with invalid arguments --\n";
$invalid_args = array (
  "string",
  10,
  10.5,
  true,
  array(1,2,3),
  new stdclass,
);
/* loop to test fwrite() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  var_dump( fwrite($invalid_args[$loop_counter - 1], 10) );
}

// fwrite() on a file handle which is already closed 
echo "-- Testing fwrite() with closed/unset file handle --\n";
fclose($file_handle);
var_dump(fwrite($file_handle,"data"));

// fwrite on a file handle which is unset 
$fp = fopen($filename, "w");
unset($fp); //unset file handle 
var_dump( fwrite(@$fp,"data"));

echo "Done\n";
?>
<?php
$filename = dirname(__FILE__)."/fwrite_error.tmp";
unlink( $filename );
?>
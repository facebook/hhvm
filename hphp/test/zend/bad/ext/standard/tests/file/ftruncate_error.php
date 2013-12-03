<?php
/*
 Prototype: bool ftruncate ( resource $handle, int $size );
 Description: truncates a file to a given length
*/

echo "*** Testing ftruncate() : error conditions ***\n";

$filename = dirname(__FILE__)."/ftruncate_error.tmp";
$file_handle = fopen($filename, "w" );
fwrite($file_handle, (binary)"Testing ftruncate error conditions \n");
fflush($file_handle);
echo "\n Initial file size = ".filesize($filename)."\n";

echo "-- Testing ftruncate() with less than expected number of arguments --\n";

// zero arguments 
var_dump( ftruncate() );

// arguments less than expected numbers
var_dump( ftruncate( $file_handle ) );
// check the first size 
var_dump( filesize($filename) );

echo "-- Testing ftruncate() with more than expected number of arguments --\n";
// more than expected number of arguments 
var_dump( ftruncate($file_handle, 10, 20) );
// check the first size 
var_dump( filesize($filename) );

// test invalid arguments : non-resources
echo "-- Testing ftruncate() with invalid file pointer --\n";
$invalid_args = array (
  "string",
  10,
  10.5,
  true,
  array(1,2,3),
  new stdclass,
);
/* loop to test ftruncate() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  var_dump( ftruncate($invalid_args[$loop_counter - 1], 10) );
}

// ftruncate() on a file handle which is already closed/unset
echo "-- Testing ftruncate() with closed/unset file handle --\n";

// ftruncate on close file handle
fclose($file_handle);
var_dump( ftruncate($file_handle,10) );
// check the first size 
var_dump( filesize($filename) );

// ftruncate on a file handle which is unset
$fp = fopen($filename, "w");
unset($fp); //unset file handle
var_dump( ftruncate(@$fp,10));
// check the first size 
var_dump( filesize($filename) );

echo "Done\n";
?> 
<?php
$filename = dirname(__FILE__)."/ftruncate_error.tmp";
unlink( $filename );
?>
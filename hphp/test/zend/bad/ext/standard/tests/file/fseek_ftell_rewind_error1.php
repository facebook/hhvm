<?php

/* Prototype: int fseek ( resource $handle, int $offset [, int $whence] );
   Description: Seeks on a file pointer

   Prototype: bool rewind ( resource $handle );
   Description: Rewind the position of a file pointer

   Prototype: int ftell ( resource $handle );
   Description: Tells file pointer read/write position
*/

echo "*** Testing fseek() : error conditions ***\n";
// zero argument
echo "-- Testing fseek() with zero argument --\n";
var_dump( fseek() );

// unexpected no. of args
echo "-- Testing fseek() with unexpected number of arguments --\n";
$fp = fopen(__FILE__, "r");
var_dump( fseek($fp) );
var_dump( fseek($fp, 10, $fp,10) );

// test invalid arguments : non-resources
echo "-- Testing fseek() with invalid arguments --\n";
$invalid_args = array (
  "string",
  10,
  10.5,
  true,
  array(1,2,3),
  new stdclass
);
/* loop to test fseek() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  var_dump( fseek($invalid_args[$loop_counter - 1], 10) );
}

// fseek() on a file handle which is already closed
echo "-- Testing fseek() with closed/unset file handle --";
fclose($fp);
var_dump(fseek($fp,10));

// fseek() on a file handle which is unset
$file_handle = fopen(__FILE__, "r");
unset($file_handle); //unset file handle
var_dump( fseek(@$file_handle,10));

echo "Done\n";
?>
<?php
/*
 Prototype: string fgets ( resource $handle [, int $length] );
 Description: Gets line from file pointer
*/

echo "*** Testing error conditions ***\n";
// zero argument
echo "-- Testing fgets() with zero argument --\n";
var_dump( fgets() );

// more than expected no. of args
echo "-- Testing fgets() with more than expected number of arguments --\n";
$fp = fopen(__FILE__, "r");
var_dump( fgets($fp, 10, $fp) );

// invalid length argument 
echo "-- Testing fgets() with invalid length arguments --\n";
$len = 0; 
var_dump( fgets($fp, $len) );
$len = -10;
var_dump( fgets($fp, $len) );
$len = 1; 
var_dump( fgets($fp, $len) ); // return length - 1 always, expect false


// test invalid arguments : non-resources
echo "-- Testing fgets() with invalid arguments --\n";
$invalid_args = array (
  "string",
  10,
  10.5,
  true,
  array(1,2,3),
  new stdclass,
);
/* loop to test fgets() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  var_dump( fgets($invalid_args[$loop_counter - 1], 10) );
}

// fgets() on a file handle which is already closed
echo "-- Testing fgets() with closed/unset file handle --";
fclose($fp);
var_dump(fgets($fp,10));

// fgets() on a file handle which is unset
$file_handle = fopen(__FILE__, "r");
unset($file_handle); //unset file handle
var_dump( fgets(@$file_handle,10));

echo "Done\n";
?>
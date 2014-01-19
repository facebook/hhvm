<?php
/*
 Prototype: array fgetcsv ( resource $handle [, int $length [, string $delimiter [, string $enclosure [, string $escape]]]] );
 Description: Gets line from file pointer and parse for CSV fields
*/

echo "*** Testing error conditions ***\n";
// zero argument
echo "-- Testing fgetcsv() with zero argument --\n";
var_dump( fgetcsv() );

// more than expected no. of args
echo "-- Testing fgetcsv() with more than expected number of arguments --\n";
$fp = fopen(__FILE__, "r");
$len = 1024;
$delim = ";";
$enclosure ="\"";
$escape = '"';
var_dump( fgetcsv($fp, $len, $delim, $enclosure, $escape, $fp) );
fclose($fp);

// test invalid arguments : non-resources
echo "-- Testing fgetcsv() with invalid arguments --\n";
$invalid_args = array (
  "string",
  10,
  10.5,
  true,
  array(1,2,3),
  new stdclass,
);
/* loop to test fgetcsv() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  var_dump( fgetcsv($invalid_args[$loop_counter - 1]) ); // with default args
  var_dump( fgetcsv($invalid_args[$loop_counter - 1], $len, $delim, $enclosure, $escape) ); // all args specified
}

echo "Done\n";
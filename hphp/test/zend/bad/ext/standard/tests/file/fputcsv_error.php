<?php
/*
 Prototype: int fputcsv ( resource $handle [, array $fields [, string $delimiter [, string $enclosure]]] );
 Description:fputcsv() formats a line (passed as a fields array) as CSV and write it to the specified file
   handle. Returns the length of the written string, or FALSE on failure. 
*/

echo "*** Testing error conditions ***\n";
// zero argument
echo "-- Testing fputcsv() with zero argument --\n";
var_dump( fputcsv() );

// more than expected no. of args
echo "-- Testing fputcsv() with more than expected number of arguments --\n";
$fp = fopen(__FILE__, "r");
$fields = array("fld1", "fld2");
$delim = ";";
$enclosure ="\"";
var_dump( fputcsv($fp, $fields, $delim, $enclosure, $fp) );
fclose($fp);

// test invalid arguments : non-resources
echo "-- Testing fputcsv() with invalid arguments --\n";
$invalid_args = array (
  "string",
  10,
  10.5,
  true,
  array(1,2,3),
  new stdclass,
);
/* loop to test fputcsv() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  var_dump( fputcsv($invalid_args[$loop_counter - 1]) ); // with default args
  var_dump( fputcsv($invalid_args[$loop_counter - 1], $fields, $delim, $enclosure) ); // all args specified
}

echo "Done\n";
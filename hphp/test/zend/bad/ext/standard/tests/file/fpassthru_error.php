<?php
/* 
Prototype: int fpassthru ( resource $handle );
Description: Reads to EOF on the given file pointer from the current position
  and writes the results to the output buffer.
*/

echo "*** Test error conditions of fpassthru() function ***\n";

/* Non-existing file resource */
$no_file = fread("/no/such/file", "r");
var_dump( fpassthru($no_file) );

/* No.of args less than expected */
var_dump( fpassthru() );

/* No.of args greaer than expected */
var_dump( fpassthru("", "") );

echo "\n*** Done ***\n";

?>
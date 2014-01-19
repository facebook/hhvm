<?php

echo "\n*** Testing error conditions ***\n";
/* Invalid number of args */
var_dump( array_values() );  // Zero arguments
var_dump( array_values(array(1,2,3), "") );  // No. of args > expected
/* Invalid types */
var_dump( array_values("") );  // Empty string
var_dump( array_values(100) );  // Integer
var_dump( array_values(new stdclass) );  // object

echo "Done\n";
?>
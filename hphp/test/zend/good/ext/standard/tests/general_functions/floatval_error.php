<?php
/* Prototype: float floatval( mixed $var );
 * Description: Returns the float value of var.
 */

echo "*** Testing floatval() and doubleval() : error conditions ***\n";


echo "\n-- Testing floatval() and doubleval() function with no arguments --\n";
var_dump( floatval() );
var_dump( doubleval() );

echo "\n-- Testing floatval() and doubleval() function with more than expected no. of arguments --\n";
var_dump( floatval(10.5, FALSE) );
var_dump( doubleval(10.5, FALSE) );

?>
===DONE===
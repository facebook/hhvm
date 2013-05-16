<?php
/* Prototype  : string long2ip(int proper_address)
 * Description: Converts an (IPv4) Internet network address into a string in Internet standard dotted format 
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: 
 */

echo "*** Testing long2ip() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing long2ip() function with Zero arguments --\n";
var_dump( long2ip() );

//Test long2ip with one more than the expected number of arguments
echo "\n-- Testing long2ip() function with more than expected no. of arguments --\n";
$proper_address = 10;
$extra_arg = 10;
var_dump( long2ip($proper_address, $extra_arg) );

?>
===DONE===
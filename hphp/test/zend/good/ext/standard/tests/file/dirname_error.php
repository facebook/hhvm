<?php
/* Prototype  : string dirname(string path)
 * Description: Returns the directory name component of the path 
 * Source code: ext/standard/string.c
 * Alias to functions: 
 */

echo "*** Testing dirname() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing dirname() function with Zero arguments --\n";
var_dump( dirname() );

//Test dirname with one more than the expected number of arguments
echo "\n-- Testing dirname() function with more than expected no. of arguments --\n";
$path = 'string_val';
$extra_arg = 10;
var_dump( dirname($path, $extra_arg) );

?>
===DONE===
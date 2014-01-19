<?php
/* Prototype  : mixed array_reduce(array input, mixed callback [, int initial])
 * Description: Iteratively reduce the array to a single value via the callback. 
 * Source code: ext/standard/array.c
 * Alias to functions: 
 */

echo "*** Testing array_reduce() : error conditions ***\n";


//Test array_reduce with one more than the expected number of arguments
echo "\n-- Testing array_reduce() function with more than expected no. of arguments --\n";
$input = array(1, 2);
$callback = 1;
$initial = 10;
$extra_arg = 10;
var_dump( array_reduce($input, $callback, $initial, $extra_arg) );

// Testing array_reduce with one less than the expected number of arguments
echo "\n-- Testing array_reduce() function with less than expected no. of arguments --\n";
$input = array(1, 2);
var_dump( array_reduce($input) );

?>
===DONE===
<?php
/* Prototype  : proto array compact(mixed var_names [, mixed ...])
 * Description: Creates a hash containing variables and their values 
 * Source code: ext/standard/array.c
 * Alias to functions: 
 */

/*
 * Error -tests test compact with zero arguments.
 */

echo "*** Testing compact() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing compact() function with Zero arguments --\n";
var_dump( compact() );


echo "Done";
?>

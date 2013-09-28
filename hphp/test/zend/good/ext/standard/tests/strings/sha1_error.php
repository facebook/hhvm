<?php


/* Prototype: string sha1  ( string $str  [, bool $raw_output  ] )
 * Description: Calculate the sha1 hash of a string
 */
 
echo "*** Testing sha1() : error conditions ***\n";

echo "\n-- Testing sha1() function with no arguments --\n";
var_dump( sha1() );

echo "\n-- Testing sha1() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( sha1("Hello World",  true, $extra_arg) );


?>
===DONE===
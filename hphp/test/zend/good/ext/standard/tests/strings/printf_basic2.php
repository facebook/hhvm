<?php
/* Prototype  : int printf  ( string $format  [, mixed $args  [, mixed $...  ]] )
 * Description: Produces output according to format .
 * Source code: ext/standard/formatted_print.c
 */

echo "*** Testing printf() : basic functionality - using integer format ***\n";


// Initialise all required variables
$format = "format";
$format1 = "%d";
$format2 = "%d %d";
$format3 = "%d %d %d";
$arg1 = 111;
$arg2 = 222;
$arg3 = 333;

echo "\n-- Calling printf() with no arguments --\n"; 
$result = printf($format);
echo "\n";
var_dump($result);

echo "\n-- Calling printf() with one arguments--\n";
$result = printf($format1, $arg1);
echo "\n";
var_dump($result);

echo "\n-- Calling printf() with two arguments--\n";
$result = printf($format2, $arg1, $arg2);
echo "\n";
var_dump($result);

echo "\n-- Calling printf() with three arguments--\n";
$result = printf($format3, $arg1, $arg2, $arg3);
echo "\n";
var_dump($result);

?>
===DONE===
<?php
/* Prototype  : int printf  ( string $format  [, mixed $args  [, mixed $...  ]] )
 * Description: Produces output according to format .
 * Source code: ext/standard/formatted_print.c
 */

echo "*** Testing printf() : basic functionality - using hexadecimal format ***\n";

// Initialise all required variables

// Initialising different format strings
$format = "format";
$format1 = "%x";
$format2 = "%x %x";
$format3 = "%x %x %x";

$format11 = "%X";
$format22 = "%X %X";
$format33 = "%X %X %X";

$arg1 = 11;
$arg2 = 132;
$arg3 = 177;

echo "\n-- Calling printf() with no arguments --\n"; 
$result = printf($format);
echo "\n";
var_dump($result);

echo "\n-- Calling printf() with one arguments --\n"; 
$result = printf($format1, $arg1);
echo "\n";
var_dump($result);
$result = printf($format11, $arg1);
echo "\n";
var_dump($result);

echo "\n-- Calling printf() with two arguments --\n"; 
$result = printf($format2, $arg1, $arg2);
echo "\n";
var_dump($result);
$result = printf($format22, $arg1, $arg2);
echo "\n";
var_dump($result);

echo "\n-- Calling printf() with three arguments --\n"; 
$result = printf($format3, $arg1, $arg2, $arg3);
echo "\n";
var_dump($result);
$result = printf($format33, $arg1, $arg2, $arg3);
echo "\n";
var_dump($result);

?>
===DONE===
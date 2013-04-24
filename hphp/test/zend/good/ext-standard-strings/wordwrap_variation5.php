<?php
/* Prototype  : string wordwrap ( string $str [, int $width [, string $break [, bool $cut]]] )
 * Description: Wraps buffer to selected number of characters using string break char
 * Source code: ext/standard/string.c
*/

/*
 *test wordwrap() with break arguments as single spaces
*/

echo "*** Testing wordwrap() : usage variations ***\n";

// Initialize all required variables
$str = "Testing wordrap function";
$width = 1;
$cut = false;

echo "\n-- Testing wordwrap() with default break value and single space as value --\n";
echo "-- with default break and cut value --\n";
var_dump( wordwrap($str, $width) );  // default break and cut value

echo "-- with default cut value --\n";
$break = ' ';
$break1 = "  ";
var_dump( wordwrap($str, $width, $break) );
var_dump( wordwrap($str, $width, $break1) );

echo "-- with cut value as false --\n";
$cut = false;
var_dump( wordwrap($str, $width, $break, $cut) );
var_dump( wordwrap($str, $width, $break1, $cut) );

echo "-- with cut value as true --\n";
$cut = true;
var_dump( wordwrap($str, $width, $break, $cut) );
var_dump( wordwrap($str, $width, $break1, $cut) );
  
echo "Done\n";
?>
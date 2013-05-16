<?php
/* Prototype  : int imagecolorstotal  ( resource $image  )
 * Description: Find out the number of colors in an image's palette
 * Source code: ext/gd/gd.c
 */

echo "*** Testing imagecolorstotal() : error conditions ***\n";

// Get a resource
$im = fopen(__FILE__, 'r');

echo "\n-- Testing imagecolorstotal() function with Zero arguments --\n";
var_dump( imagecolorstotal() );

echo "\n-- Testing imagecolorstotal() function with more than expected no. of arguments --\n";
$extra_arg = false;
var_dump( imagecolorstotal($im, $extra_arg) );

echo "\n-- Testing imagecolorstotal() function with a invalid resource\n";
var_dump( imagecolorstotal($im) );

fclose($im); 
?>
===DONE===
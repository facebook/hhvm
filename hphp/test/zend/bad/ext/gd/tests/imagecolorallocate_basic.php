<?php
/* Prototype  : int imagecolorallocate(resource im, int red, int green, int blue)
 * Description: Allocate a color for an image
 * Source code: ext/gd/gd.c
 */

echo "*** Testing imagecolorallocate() : basic functionality ***\n";

$im = imagecreatetruecolor(200, 200);
// Calling imagecolorallocate() with all possible arguments
var_dump( imagecolorallocate($im, 255, 0, 0) );
var_dump( imagecolorallocate($im, 0, 255, 0) );
var_dump( imagecolorallocate($im, 0, 0, 255) );
var_dump( imagecolorallocate($im, 255, 255, 255) );
?>
===DONE===
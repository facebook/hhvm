<?php
/* Prototype  : int imagecolorstotal(resource im)
 * Description: Find out the number of colors in an image's palette 
 * Source code: ext/gd/gd.c
 * Alias to functions: 
 */

echo "*** Testing imagecolorstotal() : basic functionality ***\n";

// Palette image
$img = imagecreate( 50, 50 );
var_dump( imagecolorstotal( $img ) );
$bg = imagecolorallocate( $img, 255, 255, 255 );
var_dump( imagecolorstotal( $img ));
$bg = imagecolorallocate( $img, 255, 0, 0 );
$bg = imagecolorallocate( $img, 0, 0, 255 );
var_dump( imagecolorstotal( $img ));
imagedestroy( $img );

// Truecolor image
$img = imagecreatetruecolor( 50, 50 );
var_dump( imagecolorstotal( $img ) );
$bg = imagecolorallocate( $img, 255, 255, 255 );
var_dump( imagecolorstotal( $img ) );
imagedestroy( $img );

?>
===DONE===
<?php
/* Prototype  : int imagecolorstotal  ( resource $image  )
 * Description: Find out the number of colors in an image's palette
 * Source code: ext/gd/gd.c
 */

echo "*** Testing imagecolorstotal() : basic functionality ***\n";

// Get an image 
$gif = dirname(__FILE__)."/php.gif";
$im = imagecreatefromgif($gif);

echo 'Total colors in image: ' . imagecolorstotal($im);

// Free image
imagedestroy($im);
?>

===DONE===
<?php

/* Prototype  : bool imagepolygon  ( resource $image  , array $points  , int $num_points  , int $color  )
 * Description: Draws a polygon.
 * Source code: ext/standard/image.c
 * Alias to functions: 
 */


echo "Simple test of imagepolygon() function\n";

$dest = dirname(realpath(__FILE__)) . '/imagepolygon.png';

// create a blank image
$image = imagecreatetruecolor(400, 300);

// set the background color to black 
$bg = imagecolorallocate($image, 0, 0, 0);

// draw a red polygon
$col_poly = imagecolorallocate($image, 255, 0, 0);

// draw the polygon
imagepolygon($image, array (
        0,   0,
        100, 200,
        300, 200
    ),
    3,
    $col_poly);

// output the picture to a file
imagepng($image, $dest);

$col1 = imagecolorat($image, 100, 200);
$col2 = imagecolorat($image, 100, 100);
$color1 = imagecolorsforindex($image, $col1);
$color2 = imagecolorsforindex($image, $col2);
var_dump($color1, $color2);
 
imagedestroy($image);

echo "Done\n"; 
?>
<?php
	$dest = dirname(realpath(__FILE__)) . '/imagepolygon.png';
	@unlink($dest);
?>	
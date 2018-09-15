<?php


<<__EntryPoint>>
function main_1781() {
$image = imagecreatetruecolor(180,40);
// Writes the text and apply a gaussian blur on the image
imagestring($image, 5, 10, 8, 'Gaussian Blur Text', 0x00ff00);
$gaussian = array(
  array(1.0, 2.0, 1.0),
  array(2.0, 4.0, 2.0),
  array(1.0, 2.0, 1.0)
);
imageconvolution($image, $gaussian, 16, 0);
// Rewrites the text for comparison
imagestring($image, 5, 10, 18, 'Gaussian Blur Text', 0x00ff00);
ob_start();
imagepng($image, null, 9);
$md5 = md5(ob_get_clean());
echo "md5: $md5\n";
}

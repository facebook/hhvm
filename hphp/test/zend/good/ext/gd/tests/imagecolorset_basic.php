<?php
// Create a 300x100 image
$im = imagecreate(300, 100);

// Set the background to be red
imagecolorallocate($im, 255, 0, 0);

// Get the color index for the background
$bg = imagecolorat($im, 0, 0);

// Set the backgrund to be blue
imagecolorset($im, $bg, 0, 0, 255);

// Get output and generate md5 hash
ob_start();
imagepng($im, null, 9);
$result_image = ob_get_contents();
ob_end_clean();
echo md5(base64_encode($result_image));
imagedestroy($im);
?>
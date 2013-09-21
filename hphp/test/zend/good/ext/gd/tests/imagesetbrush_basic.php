<?php
// Create the brush image
$img = imagecreate(10, 10);
 
// Create the main image, 100x100
$mainimg = imagecreatetruecolor(100, 100);
  
$white = imagecolorallocate($img, 255, 0, 0);
imagefilledrectangle($img, 0, 0, 299, 99, $white);

// Set the brush
imagesetbrush($mainimg, $img);
   
// Draw a couple of brushes, each overlaying each
imageline($mainimg, 50, 50, 50, 60, IMG_COLOR_BRUSHED);

// Get output and generate md5 hash
ob_start();
imagepng($mainimg, null, 9);
$result_image = ob_get_contents();
ob_end_clean();
echo md5(base64_encode($result_image));
?>
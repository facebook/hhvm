<?php

$image = imagecreatetruecolor(100, 100);

$white = imagecolorallocate($image, 0xFF, 0xFF, 0xFF);

//create an arc and fill it with white color    
imagefilledarc($image, 50, 50, 30, 30, -25, 25, $white, IMG_ARC_PIE);

ob_start();
imagepng($image);
$img = ob_get_contents();
ob_end_clean();

echo md5(base64_encode($img));
?>
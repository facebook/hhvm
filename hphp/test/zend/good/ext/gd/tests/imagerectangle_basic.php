<?php
// Create a image 
$image = imagecreatetruecolor( 100, 100 ); 

// Draw a rectangle
imagerectangle( $image, 0, 0, 50, 50, imagecolorallocate($image, 255, 255, 255) );

ob_start(); 
imagepng( $image, null, 9 ); 
$img = ob_get_contents(); 
ob_end_clean();

echo md5(base64_encode($img));

?> 
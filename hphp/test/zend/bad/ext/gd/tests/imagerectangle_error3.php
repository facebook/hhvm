<?php
// Create a image 
$image = imagecreatetruecolor( 100, 100 ); 

// Draw a rectangle
imagerectangle( $image, 'wrong param', 0, 50, 50, imagecolorallocate($image, 255, 255, 255) );
?> 
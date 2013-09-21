<?php
// Create a image 
$image = imagecreatetruecolor( 100, 100 ); 

// Draw a rectangle
imagerectangle( $image, 0, 0, 'wrong param', 50, imagecolorallocate($image, 255, 255, 255) );
?> 
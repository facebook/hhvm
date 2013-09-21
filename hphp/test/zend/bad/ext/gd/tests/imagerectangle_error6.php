<?php
// Create a image 
$image = imagecreatetruecolor( 100, 100 ); 

// Draw a rectangle
imagerectangle( $image, 0, 0, 50, 'wrong param', imagecolorallocate($image, 255, 255, 255) );
?> 
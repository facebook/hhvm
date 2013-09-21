<?php
// Create a image 
$image = imagecreatetruecolor( 100, 100 ); 

// Draw a rectangle
imagerectangle( 'wrong param', 0, 0, 50, 50, imagecolorallocate($image, 255, 255, 255) );
?> 
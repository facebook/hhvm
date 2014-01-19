<?php

// Create a blank image and add some text
$im = imagecreatetruecolor(120, 20);
$text_color = imagecolorallocate($im, 233, 14, 91);
imagestring($im, 1, 5, 5,  'A Simple Text String', $text_color);
// Set the content type header - in this case image/jpeg
header('Content-type: image/jpeg');
// Skip the filename parameter using NULL, then set the quality to 75%
imagejpeg($im, NULL, 75);
// Free up memory
imagedestroy($im);

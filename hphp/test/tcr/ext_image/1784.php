<?php

 // Create a new image instance$im = imagecreatetruecolor(100, 100);// Make the background whiteimagefilledrectangle($im, 0, 0, 99, 99, 0xFFFFFF);// Draw a text string on the imageimagestring($im, 3, 40, 20, 'GD Library', 0xFFBA00);// Output the image to browserheader('Content-type: image/gif');imagegif($im);imagedestroy($im);
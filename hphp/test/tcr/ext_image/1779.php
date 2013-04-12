<?php

 // Create a 55x30 image$im = imagecreatetruecolor(55, 30);$red = imagecolorallocate($im, 255, 0, 0);$black = imagecolorallocate($im, 0, 0, 0);// Make the background transparentimagecolortransparent($im, $black);// Draw a red rectangleimagefilledrectangle($im, 4, 4, 50, 25, $red);// Save the imageimagepng($im, './imagecolortransparent.png');imagedestroy($im);
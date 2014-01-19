<?php

$im = imagecreatetruecolor( 99, 99 ); 

imagesetpixel($im, 0, 0, 0xFF);
imagesetpixel($im, 0, 98, 0x00FF00);
imagesetpixel($im, 98, 0, 0xFF0000);
imagesetpixel($im, 98, 98, 0x0000FF);

imageflip($im, IMG_FLIP_HORIZONTAL);
imageflip($im, IMG_FLIP_VERTICAL);
imageflip($im, IMG_FLIP_BOTH);

var_dump(dechex(imagecolorat($im, 0, 0)));
var_dump(dechex(imagecolorat($im, 0, 98)));
var_dump(dechex(imagecolorat($im, 98, 0)));
var_dump(dechex(imagecolorat($im, 98, 98)));
?> 
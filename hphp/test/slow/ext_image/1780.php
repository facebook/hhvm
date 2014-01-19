<?php

$image = imagecreatefromgif(__DIR__.'/images/php.gif');
$emboss = array(array(2, 0, 0), array(0, -1, 0), array(0, 0, -1));
imageconvolution($image, $emboss, 1, 127);
header('Content-Type: image/png');
ob_start();
imagepng($image, null, 9);
var_dump(substr(ob_get_clean(),0,10));

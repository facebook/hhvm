<?php

$sqrt3 = sqrt(3);

$pixel = new ImagickPixel('red');
var_dump($pixel->isPixelSimilar($pixel, 0));
var_dump($pixel->isPixelSimilar('red', 0));
var_dump($pixel->isPixelSimilar('#FF0000', 0));
var_dump($pixel->isPixelSimilar('rgb(255, 0, 0)', 0));
var_dump($pixel->isPixelSimilar('green', 0.01));
var_dump($pixel->isPixelSimilar('#0000FF', 0.02));
var_dump($pixel->isPixelSimilar('rgb(0, 0, 0)', 0.03));

$pixel = new ImagickPixel('#F02B88');
var_dump($pixel->isPixelSimilar('#F02B89', 0.9 / $sqrt3 / 255));
var_dump($pixel->isPixelSimilar('#F02B89', 1.1 / $sqrt3 / 255));
var_dump($pixel->isPixelSimilar('#F12A86', 0.9 / 255));
var_dump($pixel->isPixelSimilar('#F12A88', 1.1 / 255));
?>
==DONE==

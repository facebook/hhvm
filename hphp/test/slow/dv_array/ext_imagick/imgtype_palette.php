<?php

$im = new Imagick;
$im->readImage(__DIR__.'/facebook.png');

if ($im->getImageType() === Imagick::IMGTYPE_PALETTE)
    echo "this image has a palette\n";
else
    echo "this image has no palette\n";

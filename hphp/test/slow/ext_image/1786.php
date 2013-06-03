<?php

// Create an image instance
$im = imagecreatefromgif(__DIR__.'/images/php.gif');
// Enable interlancing
imageinterlace($im, true);
// Save the interfaced image
imagegif($im);
imagedestroy($im);

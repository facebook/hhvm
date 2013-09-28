<?php

// File and new size
$filename = __DIR__.'/../../images/simpletext.jpg';
$percent = 0.5;
// Content type
header('Content-type: image/jpeg');
// Get new sizes
list($width, $height) = getimagesize($filename, $nfo);
var_dump($nfo);
$newwidth = $width * $percent;
$newheight = $height * $percent;
// Load
$thumb = imagecreatetruecolor($newwidth, $newheight);
$source = imagecreatefromjpeg($filename);
// Resize
imagecopyresized($thumb, $source, 0, 0, 0, 0, $newwidth, $newheight, $width, $height);
// Output
imagejpeg($thumb);

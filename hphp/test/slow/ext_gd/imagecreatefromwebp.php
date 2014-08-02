<?php
$filename = dirname(__FILE__).'/imagecreatefromwebp.webp';

if (!file_exists($filename))
  die("$filename does not exist. This test case needs a webp-image.");

$im = imagecreatefromwebp($filename);

// Convert it to a high quality png file
imagepng($im, "$filename.png", 9);
imagedestroy($im);

$fp = fopen( "$filename.png", 'r');

// We want to be sure that there really is a png file
// with a reasonable file size.
echo floor(filesize($filename)/100)*100, "\n";
$data = fread($fp, 4);

// remove non-ASCII chars
echo preg_replace('/[[:^print:]]/', '?', $data);


unlink("$filename.png");

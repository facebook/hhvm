<?php

echo "TC IMG_CROP_DEFAULT\n";
$im = imagecreatetruecolor(99, 99); 
imagefilledrectangle($im, 20, 20, 30, 30, 0xff);
$im_crop = imagecropauto($im, IMG_CROP_DEFAULT);
var_dump(imagesx($im_crop));
var_dump(imagesy($im_crop));

echo "Palette IMG_CROP_DEFAULT\n";
$im = imagecreate(99, 99); 
imagefilledrectangle($im, 20, 20, 30, 30, 0xff);
$im_crop = imagecropauto($im, IMG_CROP_DEFAULT);
var_dump(imagesx($im_crop));
var_dump(imagesy($im_crop));

echo "TC IMG_CROP_SIDES\n";
$im = imagecreatetruecolor(99, 99); 
imagefilledrectangle($im, 20, 20, 30, 30, 0xff);
$im_crop = imagecropauto($im, IMG_CROP_SIDES);
var_dump(imagesx($im_crop));
var_dump(imagesy($im_crop));

echo "Palette IMG_CROP_SIDES\n";
$im = imagecreate(99, 99); 
imagefilledrectangle($im, 20, 20, 30, 30, 0xff);
$im_crop = imagecropauto($im, IMG_CROP_SIDES);
var_dump(imagesx($im_crop));
var_dump(imagesy($im_crop));

echo "TC IMG_CROP_BLACK\n";
$im = imagecreatetruecolor(50, 50);
imagefilledrectangle($im, 20, 20, 30, 30, 0xff);
$im_crop = imagecropauto($im, IMG_CROP_BLACK);
var_dump(imagesx($im_crop));
var_dump(imagesy($im_crop));

echo "Palette IMG_CROP_BLACK\n";
$im = imagecreate(50, 50);
$bgd = imagecolorallocate($im, 0, 0, 0);
$b = imagecolorallocate($im, 0, 0, 255);
imagefilledrectangle($im, 20, 20, 30, 30, 0xff);
$im_crop = imagecropauto($im, IMG_CROP_BLACK);
var_dump(imagesx($im_crop));
var_dump(imagesy($im_crop));

echo "IMG_CROP_THRESHOLD\n";
$im = imagecreatefrompng(__DIR__ . "/logo_noise.png");
$im_crop = imagecropauto($im, IMG_CROP_THRESHOLD, 0.1, 0x0);
imagepng($im_crop, __DIR__ . "/crop_threshold.png");
var_dump(imagesx($im_crop));
var_dump(imagesy($im_crop));

@unlink(__DIR__ . "/crop_threshold.png");
?> 
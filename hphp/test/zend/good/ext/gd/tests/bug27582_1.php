<?php
/* $id$ */
$dest = dirname(realpath(__FILE__)) . '/bug27582.png';
@unlink($dest);
$im = ImageCreateTrueColor(10, 10);
imagealphablending($im, true);
imagesavealpha($im, true);
$bordercolor=ImageColorAllocateAlpha($im, 0, 0, 0, 2);
$color = ImageColorAllocateAlpha($im, 0, 0, 0, 1);
ImageFillToBorder($im, 5, 5, $bordercolor, $color);
imagepng($im, $dest);

$im2 = imagecreatefrompng($dest);
$col = imagecolorat($im2, 5, 5);
$color = imagecolorsforindex($im2, $col);
echo $color['alpha'];
@unlink($dest);
?>
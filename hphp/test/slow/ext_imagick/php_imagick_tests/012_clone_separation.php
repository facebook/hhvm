<?php
$im = new Imagick();
$im->newImage(100, 100, new ImagickPixel("white"));

$new = clone $im;
$new->thumbnailImage(200, null);
var_dump($im->width, $new->width);

// In hphp, `clone` is a reserved word, and following code will be syntax error
// $new2 = $im->clone();
// $new2->thumbnailImage(200, null);
// var_dump($im->width, $new2->width);
?>

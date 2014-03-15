<?php

/* Generate array of pixels. 2000 pixels per color stripe */
$count = 2000 * 3;

$pixels =
   array_merge(array_pad(array(), $count, 0),
               array_pad(array(), $count, 255),
               array_pad(array(), $count, 0),
               array_pad(array(), $count, 255),
               array_pad(array(), $count, 0));

/* Width and height. The area is amount of pixels divided
   by three. Three comes from 'RGB', three values per pixel */
$width = $height = 100;

/* Create empty image */
$im = new Imagick();
$im->newImage($width, $height, 'gray');

/* Import the pixels into image.
   width * height * strlen("RGB") must match count($pixels) */
$im->importImagePixels(0, 0, $width, $height, "RGB", Imagick::PIXEL_CHAR, $pixels);

var_dump($width, $height);
var_dump($im->getImageGeometry());

?>

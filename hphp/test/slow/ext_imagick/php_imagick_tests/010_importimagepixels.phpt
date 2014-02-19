--TEST--
Test importimagepixels
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc');

if (!method_exists("imagick", "importimagepixels")) {
  die("skip imagick::importImagePixels not available");
}

?>
--FILE--
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
--EXPECTF--
int(100)
int(100)
array(2) {
  ["width"]=>
  int(100)
  ["height"]=>
  int(100)
}

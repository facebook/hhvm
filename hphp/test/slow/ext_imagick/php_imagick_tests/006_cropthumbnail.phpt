--TEST--
Test cropthumbnail
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php

$im = new Imagick("magick:rose");
$im->cropThumbnailImage(200, 200);
var_dump($im->getImageGeometry());

$im = new Imagick("magick:rose");
$im->cropThumbnailImage(170, 120);
var_dump($im->getImageGeometry());

$im = new Imagick("magick:rose");
$im->cropThumbnailImage(50, 50);
var_dump($im->getImageGeometry());

$im = new Imagick("magick:rose");
$im->cropThumbnailImage(60, 120);
var_dump($im->getImageGeometry());

$im = new Imagick("magick:logo");
$im->cropThumbnailImage(100, 100);
var_dump($im->getImageGeometry());

$im = new Imagick("magick:rose");
$im->cropThumbnailImage(200, 10);
var_dump($im->getImageGeometry());

?>
--EXPECTF--
array(2) {
  ["width"]=>
  int(200)
  ["height"]=>
  int(200)
}
array(2) {
  ["width"]=>
  int(170)
  ["height"]=>
  int(120)
}
array(2) {
  ["width"]=>
  int(50)
  ["height"]=>
  int(50)
}
array(2) {
  ["width"]=>
  int(60)
  ["height"]=>
  int(120)
}
array(2) {
  ["width"]=>
  int(100)
  ["height"]=>
  int(100)
}
array(2) {
  ["width"]=>
  int(200)
  ["height"]=>
  int(10)
}

<?php

$im = new Imagick();
$im->newPseudoImage(100, 100, "XC:red");
var_dump($im->getImageGeometry());

$im->newPseudoImage(0, 0, "magick:logo");
var_dump($im->getImageGeometry());

$im->readImage("magick:logo");
var_dump($im->getImageGeometry());

$im->newPseudoImage(0, 0, "rose:");
var_dump($im->getImageGeometry());

try {
$im->newPseudoImage(0, 0, "png:");
var_dump($im->getImageGeometry());
} catch (Exception $e) {
  echo "fail\n";
}

?>

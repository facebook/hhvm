--TEST--
Test pseudo formats
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
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
--EXPECTF--
array(2) {
  ["width"]=>
  int(%d)
  ["height"]=>
  int(%d)
}
array(2) {
  ["width"]=>
  int(%d)
  ["height"]=>
  int(%d)
}
array(2) {
  ["width"]=>
  int(%d)
  ["height"]=>
  int(%d)
}
array(2) {
  ["width"]=>
  int(%d)
  ["height"]=>
  int(%d)
}
fail

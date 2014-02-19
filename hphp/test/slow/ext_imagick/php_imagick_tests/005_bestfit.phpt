--TEST--
Test thumbnail bestfit
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php

$im = new Imagick();
$im->newImage(50, 100, 'white');

$im->thumbnailImage(100, 50, true);
var_dump($im->getImageGeometry());

?>
--EXPECTF--
array(2) {
  ["width"]=>
  int(25)
  ["height"]=>
  int(50)
}

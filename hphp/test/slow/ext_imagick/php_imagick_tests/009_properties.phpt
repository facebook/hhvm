--TEST--
Test reading properties
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php

$im = new Imagick();
$im->newPseudoImage(100, 100, "XC:red");
$im->setImageFormat("png");

echo $im->width . "x" . $im->height . "\n";
echo $im->format;

?>
--EXPECTF--
100x100
png

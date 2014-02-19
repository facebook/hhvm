--TEST--
Test clone length, this is expected upstream behaviour
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc');

$v = imagick::getversion ();

if ($v ['versionNumber'] >= 0x640 && $v ['versionNumber'] < 0x650)
  die ('skip seems to be different in this version of ImageMagick');
?>
--FILE--
<?php

$im = new Imagick ('magick:rose');
$im->setImageFormat ('png');
var_dump ($im->getImageLength ());

$cloned = clone $im;
$cloned->setImageFormat ('png');

var_dump ($cloned->getImageLength ());

?>
--EXPECT--
int(9673)
int(0)

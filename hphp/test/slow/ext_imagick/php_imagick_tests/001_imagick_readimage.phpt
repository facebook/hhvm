--TEST--
Imagick::readImage test
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php
print "--- Catch exception with try/catch\n";
$imagick = new Imagick();
try {
  $imagick->readImage('foo.jpg');
} catch (ImagickException $e) {
  echo "got exception";
}

?>
--EXPECTF--
--- Catch exception with try/catch
got exception

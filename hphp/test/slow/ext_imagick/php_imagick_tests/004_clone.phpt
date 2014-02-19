--TEST--
Testing clone keyword
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php
print "--- Testing clone keyword\n";

try {
  $im = new Imagick();
  $im->newImage(100, 100, new ImagickPixel("white"));
  $new = clone $im;

  if ($new->getImageWidth() == 100 && $new->getImageHeight() == 100) {
    echo "Cloning succeeded\n";
  } else {
    echo "Cloning failed\n";
  }
} catch (Exception $e) {
  echo "Cloning failed\n";
}
?>
--EXPECTF--
--- Testing clone keyword
Cloning succeeded

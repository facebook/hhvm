<?php

try {
  $pixel = new ImagickPixel('the-best-color-in-the-world');
} catch (ImagickPixelException $ex) {
  echo "__construct\n";
}

$pixel = new ImagickPixel('white');

try {
  $pixel->setColor('the-worst-color-in-the-world');
} catch (ImagickPixelException $ex) {
  echo "setColor\n";
}

define('IMAGICK_COLOR_INVALID', -1);

try {
  $pixel->getColorValue(IMAGICK_COLOR_INVALID);
} catch (ImagickPixelException $ex) {
  echo "getColorValue\n";
}

try {
  $pixel->setColorValue(IMAGICK_COLOR_INVALID, 0);
} catch (ImagickPixelException $ex) {
  echo "setColorValue\n";
}

try {
  $pixel->isPixelSimilar(new ImagickPixelException, 0);
} catch (ImagickPixelException $ex) {
  echo "isPixelSimilar\n";
}
?>
==DONE==

<?php

$im = new Imagick("magick:logo");
$im->setImageBackgroundColor("pink");
$im->thumbnailImage(200, 200, true, true);

$color = $im->getImagePixelColor(5, 5);
if ($color->isPixelSimilar("pink", 0))
  echo "Similar" . PHP_EOL;
else
  var_dump ($color->getColorAsString());

$color = $im->getImagePixelColor(199, 5);
if ($color->isPixelSimilar("pink", 0))
  echo "Similar" . PHP_EOL;
else
  var_dump ($color->getColorAsString());
?>

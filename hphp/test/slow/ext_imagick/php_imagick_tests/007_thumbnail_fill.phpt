--TEST--
Test filling thumbnail with color
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc');

$v = Imagick::getVersion();
if ($v['versionNumber'] < 0x632)
  die ('skip too old ImageMagick');

if ($v ['versionNumber'] >= 0x660 && $v ['versionNumber'] < 0x670)
  die ('skip seems to be broken in this version of ImageMagick');
?>
--FILE--
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
--EXPECT--
Similar
Similar

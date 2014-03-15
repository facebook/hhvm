<?php
function test($magick) {
  print count($magick)."\n";
  foreach ($magick as $k => $v) {
    printf("[%s] %sx%s.%s\n", $k, $v->width, $v->height, $v->format);
  }
}

$img = array(
  'magick:logo',
  __DIR__.'/facebook.png',
  __DIR__.'/draw_example.png',
  __DIR__.'/php_imagick_tests/php.gif',
);

$magick = new Imagick($img);
foreach ($img as $i) {
  $magick->readImage($i);
}
$magick->readImages($img);
$magick->removeImage();
test($magick);

$magick->setFirstIterator();
$magick->removeImage();
$magick->setLastIterator();
$magick->removeImage();
test($magick);

$magick->setIteratorIndex(4);
$magick->removeImage();
test($magick);


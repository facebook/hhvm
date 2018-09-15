<?php

$magick = new Imagick(__DIR__.'/facebook.png');
$iterator = new ImagickPixelIterator($magick);

foreach ($iterator as $row => $pixels) {
  $col = count($pixels);
  $color = $pixels[$row % $col]->getColor();
  printf("[%s] (%d) %02X%02X%02X\n",
    $row, $col, $color['r'], $color['g'], $color['b']);
}


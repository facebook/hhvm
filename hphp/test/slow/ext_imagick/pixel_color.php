<?php

// hphp doesn't support this at this moment
// ini_set('precision', 3);

function dump($exp) {
  $ret = print_r($exp, true);
  $ret = preg_replace_callback(
    '/\d+\.\d+/',
    function ($matches) {
      return sprintf('%.3f', $matches[0]);
    },
    $ret
  );
  echo "$ret\n";
}

$pixel = new ImagickPixel;
$pixel->setColor('yellow');
dump($pixel->getHSL());
dump($pixel->getColor(true));
$pixel = new ImagickPixel($pixel->getColorAsString());
dump($pixel->getHSL());
dump($pixel->getColor(false));

$pixel = new ImagickPixel;
$pixel->setHSL(0.3, 0.4, 0.5);
dump($pixel->getHSL());
dump($pixel->getColor(false));
$pixel = new ImagickPixel($pixel->getColorAsString());
dump($pixel->getHSL());
dump($pixel->getColor(true));

$pixel = new ImagickPixel('#F02B88');
$colors = array(
  Imagick::COLOR_BLACK,
  Imagick::COLOR_BLUE,
  Imagick::COLOR_CYAN,
  Imagick::COLOR_GREEN,
  Imagick::COLOR_RED,
  Imagick::COLOR_YELLOW,
  Imagick::COLOR_MAGENTA,
  Imagick::COLOR_ALPHA,
  Imagick::COLOR_FUZZ,
);
foreach ($colors as $color) {
  dump($pixel->getColorValue($color));
}

foreach ($colors as $color) {
  $pixel->setColorValue($color, $pixel->getColorValue($color));
}
dump($pixel->getHSL());
dump($pixel->getColor());
?>
==DONE==

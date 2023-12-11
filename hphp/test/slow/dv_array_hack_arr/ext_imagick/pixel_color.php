<?hh

// hphp doesn't support this at this moment
// ini_set('precision', 3);

function dump($exp) :mixed{
  $ret = print_r($exp, true);
  $count = -1;
  $ret = preg_replace_callback(
    '/\d+\.\d+/',
    function ($matches) {
      return sprintf('%.3f', $matches[0]);
    },
    $ret,
    -1,
    inout $count
  );
  echo "$ret\n";
}
<<__EntryPoint>> function main(): void {
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
$colors = vec[
  Imagick::COLOR_BLACK,
  Imagick::COLOR_BLUE,
  Imagick::COLOR_CYAN,
  Imagick::COLOR_GREEN,
  Imagick::COLOR_RED,
  Imagick::COLOR_YELLOW,
  Imagick::COLOR_MAGENTA,
  Imagick::COLOR_ALPHA,
  Imagick::COLOR_FUZZ,
];
foreach ($colors as $color) {
  dump($pixel->getColorValue($color));
}

foreach ($colors as $color) {
  $pixel->setColorValue($color, $pixel->getColorValue($color));
}
dump($pixel->getHSL());
dump($pixel->getColor());

echo "==DONE==\n";
}

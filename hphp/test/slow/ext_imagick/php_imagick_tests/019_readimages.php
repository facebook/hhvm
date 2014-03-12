<?php

try {
  $imagick = new Imagick(array (
              'magick:rose',
              'magick:rose',
              'fail_this_does_not_exist.jpg',
              'magick:rose',
  ));
  echo 'Fail'. PHP_EOL;
} catch (ImagickException $e) {
  echo 'OK' . PHP_EOL;
}

try {
  $imagick = new Imagick(array (
              'magick:rose',
              'magick:rose',
              'magick:rose',
  ));
  echo 'OK' . PHP_EOL;
  $imagick->readImages (array (
              'magick:rose',
              'magick:rose',
              'magick:rose',
  ));
  echo 'OK' . PHP_EOL;
  $imagick->readImages (array (
              'magick:rose',
              'fail_this_does_not_exist.jpg',
  ));
  echo 'Fail' . PHP_EOL;
} catch (ImagickException $e) {
  echo 'Third OK'. PHP_EOL;
}

?>

<?php
print "--- Catch exception with try/catch\n";
$imagick = new Imagick();
try {
  $imagick->readImage('foo.jpg');
} catch (ImagickException $e) {
  echo "got exception";
}

?>

<?php
$im = new Imagick ('magick:rose');
$im->convolveimage (array (1, 'a', 1));

echo "OK" . PHP_EOL;
?>

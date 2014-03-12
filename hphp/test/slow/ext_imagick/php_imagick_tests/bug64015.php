<?php
$im = new Imagick(__DIR__ . '/php.gif');
var_dump($im->getImageLength());

// Both should return filesize in bytes.

var_dump($im->getImageLength());

// All cases below now return 0;
$cloned_im = clone $im;
var_dump($im->getImageLength());

echo "OK" , PHP_EOL;


?>

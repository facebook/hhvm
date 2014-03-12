<?php

$im = new Imagick ('magick:rose');
$im->setImageFormat ('png');
var_dump ($im->getImageLength ());

$cloned = clone $im;
$cloned->setImageFormat ('png');

var_dump ($cloned->getImageLength ());

?>

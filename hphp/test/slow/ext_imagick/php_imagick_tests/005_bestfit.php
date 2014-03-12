<?php

$im = new Imagick();
$im->newImage(50, 100, 'white');

$im->thumbnailImage(100, 50, true);
var_dump($im->getImageGeometry());

?>

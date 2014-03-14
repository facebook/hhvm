<?php

$im = new Imagick();
$im->newPseudoImage(100, 100, "XC:red");
$im->setImageFormat("png");

echo $im->width . "x" . $im->height . "\n";
echo $im->format;

?>

<?php
$image = imagecreate(180, 30);
$white = imagecolorallocate($image, 255, 255, 255);

$totalColors = imagecolorstotal($image);

$result = imagecolordeallocate($image, -1.0);
var_dump($result);
?>
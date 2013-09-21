<?php
$image = imagecreate(180, 30);
$white = imagecolorallocate($image, 255, 255, 255);

$totalColors = imagecolorstotal($image);

$result = imagecolordeallocate($image, $totalColors + 100);
var_dump($result);
?>
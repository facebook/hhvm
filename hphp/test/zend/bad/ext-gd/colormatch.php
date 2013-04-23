<?php

$im = imagecreatetruecolor(5,5);
$im2 = imagecreate(5,5);

imagecolormatch($im, $im2);

echo "ok\n";

imagedestroy($im);
?>
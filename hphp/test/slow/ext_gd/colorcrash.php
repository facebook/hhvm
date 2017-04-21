<?php
$img=imagecreatetruecolor(10, 10);
imagetruecolortopalette($img, false, PHP_INT_MAX / 8);

$img = imagecreate(100, 100);
imageline($img, 0, 0, 100, 100, -7);
imagepolygon($img, array(10,10,10,50,50,50,50,10,10,10), 5, -7);
echo "done\n";

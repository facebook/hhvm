<?php
$filename = dirname(__FILE__) .  '/font.gdf';
$bin = "\x41\x41\x41\x41\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00";
$fp = fopen($filename, 'wb');
fwrite($fp, $bin);
fclose($fp);

$image = imagecreatetruecolor(50, 20);
$font = imageloadfont($filename);
$black = imagecolorallocate($image, 0, 0, 0);
imagestring($image, $font, 0, 0, "Hello", $black);
unlink($filename);
?>
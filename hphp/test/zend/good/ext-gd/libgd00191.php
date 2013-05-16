<?php
$im = imagecreatetruecolor(100, 100);
imagefilledrectangle($im, 0, 0, 100, 100, imagecolorallocate($im, 255, 255, 255));
imagesetthickness($im, 20);
imagefilledellipse($im, 30, 50, 20, 20, imagecolorallocate($im, 0, 0, 0));
$index = imagecolorat($im, 12, 28);
$arr = imagecolorsforindex($im, $index);
if ($arr['red'] == 255 && $arr['green'] == 255 && $arr['blue'] == 255) {
	echo "Ok";
} else {
	echo "failed";
}
?>
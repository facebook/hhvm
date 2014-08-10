<?php
$dir = dirname(__FILE__);

$im = imagecreatetruecolor(5,5);
imagefill($im, 0,0, 0xffffff);
imagesetpixel($im, 3,3, 0x0);
imagepng($im, $dir . '/tc.png');

$im_string = file_get_contents(dirname(__FILE__) . '/tc.png');
$im = imagecreatefromstring($im_string);
echo 'createfromstring truecolor png: ';
if (imagecolorat($im, 3,3) != 0x0) {
	echo 'failed';
} else {
	echo 'ok';
}
echo "\n";
unlink($dir . '/tc.png');



$im = imagecreate(5,5);
$c1 = imagecolorallocate($im, 255,255,255);
$c2 = imagecolorallocate($im, 255,0,0);
imagefill($im, 0,0, $c1);
imagesetpixel($im, 3,3, $c2);
imagepng($im, $dir . '/p.png');

$im_string = file_get_contents(dirname(__FILE__) . '/p.png');
$im = imagecreatefromstring($im_string);

echo'createfromstring palette png: ';

$c = imagecolorsforindex($im, imagecolorat($im, 3,3));
$failed = false;
if ($c['red'] != 255 || $c['green'] != 0 || $c['blue'] != 0) {
	echo 'failed';
} else {
	echo 'ok';
}
echo "\n";
unlink($dir . '/p.png');


//empty string
$im = imagecreatefromstring('');
//random string > 8
$im = imagecreatefromstring(' asdf jklp');
?>

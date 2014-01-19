<?php
$im = imagecreatetruecolor(10,10);
imagerectangle($im, 1,1, 1,1, 0xFFFFFF);
$c1 = imagecolorat($im, 1,1);
$c2 = imagecolorat($im, 1,2);
$c3 = imagecolorat($im, 2,1);
$c4 = imagecolorat($im, 2,2);
if ($c1 == 0xFFFFFF && $c2 == 0 && $c3 == 0 && $c4 == 0) {
	echo "Ok";
} else {
	echo "failed";
}
?>
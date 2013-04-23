<?php
$im = imagecreatetruecolor(3,1);
$bgcolor = imagecolorallocatealpha($im,255, 255, 0, 0);
imagefill($im,0,0,$bgcolor);
print_r(imagecolorat($im, 1,0));
?>
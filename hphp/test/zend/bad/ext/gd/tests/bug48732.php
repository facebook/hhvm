<?php
$cwd = dirname(__FILE__);
$font = "$cwd/Tuffy.ttf";
$g = imagecreate(100, 50);
$bgnd  = imagecolorallocate($g, 255, 255, 255);
$black = imagecolorallocate($g, 0, 0, 0);
$bbox  = imagettftext($g, 12, 0, 0, 20, $black, $font, "ABCEDFGHIJKLMN\nopqrstu\n");
imagepng($g, "$cwd/bug48732.png");
echo 'Left Bottom: (' . $bbox[0]  . ', ' . $bbox[1] . ')';
?>
<?php @unlink(dirname(__FILE__) . '/bug48732.png'); ?>
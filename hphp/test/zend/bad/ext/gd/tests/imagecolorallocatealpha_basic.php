<?php
$img = imagecreatetruecolor(150, 150);

$cor = imagecolorallocate($img, 50, 100, 255);
$corA = imagecolorallocatealpha($img, 50, 100, 255, 50);
//$whiteA = imagecolorallocatealpha($img, 255, 255, 255, 127);

$half =  imagefilledarc ( $img, 75, 75, 70, 70, 0, 180, $cor, IMG_ARC_PIE );
$half2 =  imagefilledarc ( $img, 75, 75, 70, 70, 180, 360, $corA, IMG_ARC_PIE );

ob_start();
imagepng($img, null, 9);
$imgsrc = ob_get_contents();
ob_end_clean();

var_dump(md5(base64_encode($imgsrc)));
var_dump($corA);
?>

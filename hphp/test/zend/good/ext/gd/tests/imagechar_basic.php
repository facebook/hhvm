<?php
$image = imagecreatetruecolor(180, 30);
$white = imagecolorallocate($image, 255,255,255);

$result = imagechar($image, 1, 5, 5, 'C', $white);

ob_start();
imagepng($image, null, 9);
$img = ob_get_contents();
ob_end_clean();

echo md5(base64_encode($img));
?>
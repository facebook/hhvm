<?php
$image = imagecreatetruecolor(180, 30);

$layer = imagelayereffect($image, IMG_EFFECT_REPLACE);

if ($layer){
	ob_start();
	imagepng($image, null, 9);
	$img = ob_get_contents();
	ob_end_clean();
}

echo md5(base64_encode($img));
?>
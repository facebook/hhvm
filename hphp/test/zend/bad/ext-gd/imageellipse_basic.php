<?php

// Create a image
$image = imagecreatetruecolor(400, 300);

// Draw a white ellipse
imageellipse($image, 200, 150, 300, 200, 16777215);

ob_start();
imagepng($image, null, 9);
$img = ob_get_contents();
ob_end_clean();

echo md5(base64_encode($img));
?>
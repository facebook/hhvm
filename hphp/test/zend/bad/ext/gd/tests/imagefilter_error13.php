<?php
$image = imagecreatetruecolor(180, 30);

var_dump(imagefilter($image, IMG_FILTER_COLORIZE, 800, 255, 255)); // Wrong value is truncated to 255
?>
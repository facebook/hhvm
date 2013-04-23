<?php
$image = imagecreatetruecolor(180, 30);

var_dump(imagefilter($image, IMG_FILTER_SMOOTH, 'wrong parameter'));
?>
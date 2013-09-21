<?php
$image = imagecreatetruecolor(180, 30);

var_dump(imagefilter($image, 'wrong parameter'));
?>
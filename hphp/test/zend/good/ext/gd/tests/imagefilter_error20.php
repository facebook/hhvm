<?php
$image = tmpfile();

var_dump(imagefilter($image, IMG_FILTER_BRIGHTNESS, 1));
?>
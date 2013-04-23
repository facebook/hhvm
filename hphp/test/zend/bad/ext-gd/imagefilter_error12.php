<?php
$image = tmpfile();

var_dump(imagefilter($image, IMG_FILTER_COLORIZE, 255, 255, 255));
?>
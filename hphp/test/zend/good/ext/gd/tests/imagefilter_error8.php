<?php
$image = tmpfile();

var_dump(imagefilter($image, IMG_FILTER_SELECTIVE_BLUR));
?>
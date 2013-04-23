<?php

$image = imagecreatetruecolor(100, 100);

var_dump(imageinterlace($image, 1));
var_dump(imageinterlace($image));
?>
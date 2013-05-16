<?php

$image = imagecreatetruecolor(100, 100);

//setting the interlace bit to on
imageinterlace($image, 1);

//setting de interlace bit to off
var_dump(imageinterlace($image, 0));
var_dump(imageinterlace($image));
?>
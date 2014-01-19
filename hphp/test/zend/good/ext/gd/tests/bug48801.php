<?php
$cwd = dirname(__FILE__);
$font = "$cwd/Tuffy.ttf";
$bbox = imageftbbox(50, 0, $font, "image");
echo '(' . $bbox[0] . ', ' . $bbox[1] . ")\n";
echo '(' . $bbox[2] . ', ' . $bbox[3] . ")\n";
echo '(' . $bbox[4] . ', ' . $bbox[5] . ")\n";
echo '(' . $bbox[6] . ', ' . $bbox[7] . ")\n";
?>
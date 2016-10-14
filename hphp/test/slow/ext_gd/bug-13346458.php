<?php
$img = imagecreatetruecolor(1, 1);
imagesetstyle($img, array());
imagesetpixel($img, 0, 0, -2);
imagedestroy($img);
echo "Done\n";

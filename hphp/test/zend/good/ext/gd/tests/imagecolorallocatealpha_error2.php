<?php
$img = imagecreatetruecolor(200, 200);

imagecolorallocatealpha($img, 'string-non-numeric', 255, 255, 50);
imagecolorallocatealpha($img, array(), 255, 255, 50);
imagecolorallocatealpha($img, tmpfile(), 255, 255, 50);
?>
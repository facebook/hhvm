<?php
$img = imagecreatetruecolor(200, 200);

imagecolorallocatealpha($img, 255, 255, 'string-non-numeric', 50);
imagecolorallocatealpha($img, 255, 255, array(), 50);
imagecolorallocatealpha($img, 255, 255, tmpfile(), 50);
?>
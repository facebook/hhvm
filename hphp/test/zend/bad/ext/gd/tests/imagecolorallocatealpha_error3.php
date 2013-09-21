<?php
$img = imagecreatetruecolor(200, 200);

imagecolorallocatealpha($img, 255, 'string-non-numeric', 255, 50);
imagecolorallocatealpha($img, 255, array(), 255, 50);
imagecolorallocatealpha($img, 255, tmpfile(), 255, 50);
?>
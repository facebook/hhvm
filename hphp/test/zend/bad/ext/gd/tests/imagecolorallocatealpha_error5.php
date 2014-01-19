<?php
$img = imagecreatetruecolor(200, 200);

imagecolorallocatealpha($img, 255, 255, 255, 'string-non-numeric');
imagecolorallocatealpha($img, 255, 255, 255, array());
imagecolorallocatealpha($img, 255, 255, 255, tmpfile());
?>
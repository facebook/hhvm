<?php

$image = imagecreatetruecolor(180, 30);
$white = imagecolorallocate($image, 255, 255, 255);

$resource = tmpfile();

$result = imagecolordeallocate($resource, $white);

?>
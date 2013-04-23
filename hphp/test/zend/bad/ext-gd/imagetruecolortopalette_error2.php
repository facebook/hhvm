<?php
$image = imagecreatetruecolor(50, 50);
$resource = tmpfile();

imagetruecolortopalette($image, $resource, 2);
imagetruecolortopalette($image, array(), 2);

?>
<?php
$image = imagecreatetruecolor(50, 50);
$resource = tmpfile();

imagetruecolortopalette($image, true, 'string');
imagetruecolortopalette($image, true, $resource);
imagetruecolortopalette($image, true, array());
imagetruecolortopalette($image, true, null);

?>
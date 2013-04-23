<?php
$resource = tmpfile();

imagetruecolortopalette($resource, true, 2);
imagetruecolortopalette('string', true, 2);
imagetruecolortopalette(array(), true, 2);
imagetruecolortopalette(null, true, 2);
?>
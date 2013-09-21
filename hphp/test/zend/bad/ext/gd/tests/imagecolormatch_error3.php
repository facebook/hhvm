<?php
$ima = imagecreatetruecolor(110, 20);
$background_color = imagecolorallocate($ima, 0, 0, 0);
$imb = imagecreatetruecolor(110, 20);
$background_color = imagecolorallocate($imb, 0, 0, 100);
var_dump(imagecolormatch($ima, $imb));
?>
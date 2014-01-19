<?php
$ima = imagecreatetruecolor(110, 20);
$background_color = imagecolorallocate($ima, 0, 0, 0);
var_dump(imagecolormatch($ima));
?>
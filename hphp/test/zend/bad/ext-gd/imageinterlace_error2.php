<?php
$image = fopen('php://stdin', 'r');
var_dump(imageinterlace($image));
?>
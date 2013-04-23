<?php

// Create a image
$image = imagecreatetruecolor(400, 300);

// try to draw a white ellipse
imageellipse($image, 'wrong param', 150, 300, 200, 16777215);

?>
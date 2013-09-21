<?php

// Create a image
$image = imagecreatetruecolor(400, 300);

// try to draw a white ellipse
imageellipse($image, 200, 150, 300, 200, 'wrong param');

?>
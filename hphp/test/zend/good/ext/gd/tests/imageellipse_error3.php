<?php

// Create a image
$image = imagecreatetruecolor(400, 300);

// try to draw a white ellipse
try { imageellipse($image, 200, 'wrong param', 300, 200, 16777215); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

?>

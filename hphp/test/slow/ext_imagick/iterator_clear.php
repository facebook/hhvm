<?php
$magick = new Imagick('magick:rose');
$iterator = new ImagickPixelIterator($magick);
$iterator->clear();
$iterator->destroy();
?>
==DONE==

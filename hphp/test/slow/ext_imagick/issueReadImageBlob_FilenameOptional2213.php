<?php

$blob = file_get_contents('slow/ext_imagick/facebook.png');

$im = new Imagick();

$im->readImageBlob($blob);

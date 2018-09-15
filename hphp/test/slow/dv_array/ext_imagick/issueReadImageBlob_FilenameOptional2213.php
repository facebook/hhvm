<?php

$blob = file_get_contents(__DIR__.'/facebook.png');

$im = new Imagick();

var_dump($im->readImageBlob($blob));

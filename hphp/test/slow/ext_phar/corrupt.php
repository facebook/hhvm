<?php

copy("composer.phar", "composer-corrupt.phar");
$fp = fopen("composer-corrupt.phar", "a+");

// We'll corrupt the 1024th byte
$byte = 1024;

// Seek and write a random byte
fseek($fp, $byte, SEEK_SET);
fwrite($fp, "f");
fclose($fp);

// Try to break stuff
require "composer-corrupt.phar";

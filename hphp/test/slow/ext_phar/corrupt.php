<?php

$corrupt = tempnam(sys_get_temp_dir(), 'composer-corrupt');

copy(__DIR__."/composer.phar", $corrupt);
// We'll corrupt the 1024th byte
$byte = 1024;

// Seek and write a random byte
$fp = fopen($corrupt, 'a+');
fseek($fp, $byte, SEEK_SET);
fwrite($fp, "f");
fclose($fp);

// Try to break stuff
require $corrupt;

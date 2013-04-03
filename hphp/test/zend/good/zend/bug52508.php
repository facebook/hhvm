<?php

$file = dirname(__FILE__) .'/bug52508.ini';

file_put_contents($file, "a = 1");

$ini_array = parse_ini_file($file, true, INI_SCANNER_RAW);
var_dump($ini_array);

unlink($file);

?>
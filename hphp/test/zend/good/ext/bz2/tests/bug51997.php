<?php

error_reporting(E_ALL);

$filename = "bug51997.bz2";
$str = "This is a test string.\n";
$bz = bzopen($filename, "w");
bzwrite($bz, $str);
bzclose($bz);

$bz = bzopen($filename, "r");
fseek($bz, 0, SEEK_CUR);
print bzread($bz, 10);
print bzread($bz);
bzclose($bz);
unlink($filename);

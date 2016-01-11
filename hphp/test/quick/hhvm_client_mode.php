<?php

$hhvm = PHP_BINARY;
$file = '/../../a/b/test.php';
$cmd = "$hhvm --no-config $file";
$out = exec($cmd);
echo $out;

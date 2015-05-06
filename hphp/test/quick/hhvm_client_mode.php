<?php

$hhvm = PHP_BINARY;
$file = '/../../a/b/test.php';
$cmd = "$hhvm $file";
$out = exec($cmd);
echo $out;

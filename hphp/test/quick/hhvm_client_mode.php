<?php

$hhvm = PHP_BINARY;
$file = '/home/sherlockhua/a/b/test.php';
$cmd = "$hhvm $file";
$out = exec($cmd);
echo $out;

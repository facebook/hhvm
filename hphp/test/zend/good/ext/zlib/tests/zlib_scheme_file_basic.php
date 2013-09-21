<?php
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
$contents = file($srcFile);
var_dump($contents);
?>
===DONE===
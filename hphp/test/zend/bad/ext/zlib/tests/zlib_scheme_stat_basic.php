<?php
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
stat($srcFile);
lstat($srcFile);
?>
===DONE===
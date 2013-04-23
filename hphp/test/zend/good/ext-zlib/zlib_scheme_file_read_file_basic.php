<?php
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
readfile($srcFile);
?>
===DONE===
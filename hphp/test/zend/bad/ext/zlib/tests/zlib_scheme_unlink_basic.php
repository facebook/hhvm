<?php
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
unlink($srcFile);
var_dump(file_exists($inputFileName));
?>
===DONE===
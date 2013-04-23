<?php
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
rename($srcFile, 'something.tmp');
var_dump(file_exists($inputFileName));
?>
===DONE===
<?php
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
$h = fopen($srcFile, 'r');
fpassthru($h);
fclose($h);
?>
===DONE===
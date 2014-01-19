<?php
$inputFileName = dirname(__FILE__)."/dir.gz";
$srcFile = "compress.zlib://$inputFileName";
var_dump(mkdir($srcFile));
var_dump(is_dir($srcFile));
var_dump(opendir($srcFile));
var_dump(rmdir($srcFile));
?>
===DONE===
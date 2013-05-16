<?php
$inputFileName = dirname(__FILE__)."/004.txt.gz";
$srcFile = "compress.zlib://$inputFileName";
echo "file_exists=";
var_dump(file_exists($srcFile));
echo "is_file=";
var_dump(is_file($srcFile));
echo "is_dir=";
var_dump(is_dir($srcFile));
echo "is_readable=";
var_dump(is_readable($srcFile));
echo "\n";
echo "filesize=";
var_dump(filesize($srcFile));
echo "filetype=";
var_dump(filetype($srcFile));
echo "fileatime=";
var_dump(fileatime($srcFile));

?>
===DONE===
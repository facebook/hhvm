<?php

// include the file.inc for Function: function create_files()
require(dirname(__FILE__) . '/file.inc');

$path = dirname(__FILE__) . '/fseek_dir_basic';
mkdir($path);
create_files($path, 3);

echo "call readdir():\n";
var_dump($dh = opendir($path));
$files = array();
while( FALSE !== ($files[] = readdir($dh)) ) {}
sort($files);
var_dump($files);
$files = array();

echo "\ncall fseek() on directory resource:\n";
var_dump(fseek($dh, 20));

echo "call readdir():\n";
while( FALSE !== ($files[] = readdir($dh)) ) {}
sort($files);
var_dump($files);
$files = array();

echo "\ncall fseek() with different arguments on directory resource:\n";
var_dump(fseek($dh, 20, SEEK_END));

echo "call readdir():\n";
while( FALSE !== ($files[] = readdir($dh)) ) {}
sort($files);
var_dump($files);

delete_files($path, 3);
closedir($dh);
var_dump(rmdir($path));

?>
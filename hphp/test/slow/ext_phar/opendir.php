<?php

var_dump(readdir());

$dir = opendir('phar://'.__DIR__."/basic.phar");
var_dump($dir);
var_dump(readdir());
var_dump(readdir($dir));

rewinddir($dir);
var_dump(readdir($dir));
var_dump(readdir());

var_dump(scandir('phar://'.__DIR__."/basic.phar/"));
var_dump(scandir('phar://'.__DIR__."/basic.phar/not_a_dir"));

var_dump(opendir('phar://'.__DIR__."/phpunit.phar/File_Iterator-1.3.3"));
$files = array(readdir(), readdir(), readdir(), readdir(), readdir());
asort($files);
var_dump(array_values($files));

$files = scandir('phar://'.__DIR__."/phpunit.phar/File_Iterator-1.3.3");
var_dump($files);

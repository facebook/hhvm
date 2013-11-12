<?php

$arc_name = dirname(__FILE__)."/bug38944.zip";
$foo = new ZipArchive;
$foo->open($arc_name, ZIPARCHIVE::CREATE);;

var_dump($foo->status);
var_dump($foo->statusSys);
var_dump($foo->numFiles);
var_dump($foo->filename);
var_dump($foo->comment);

var_dump($foo);

echo "Done\n";
?>
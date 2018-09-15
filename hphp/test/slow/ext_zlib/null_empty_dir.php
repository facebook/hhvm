<?php


<<__EntryPoint>>
function main_null_empty_dir() {
$a = new ZipArchive();
var_dump($a->open('foo.zip', ZipArchive::CREATE));
var_dump($a->addEmptyDir(null));
var_dump($a->close());
}

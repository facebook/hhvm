<?php

$tempdir = tempnam(sys_get_temp_dir(), __FILE__);
unlink($tempdir);
mkdir($tempdir);

$zip = new ZipArchive();
var_dump($zip->open(__FILE__.'.zip'));
var_dump($zip->extractTo($tempdir, 'dir/file.txt'));
var_dump($zip->close());

var_dump(file_get_contents($tempdir.'/dir/file.txt'));
?>

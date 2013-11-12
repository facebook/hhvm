<?php
$archive = new ZipArchive();
$archive->open('__test.zip', ZIPARCHIVE::CREATE);
var_dump($archive->addEmptyDir('test'));
print_r($archive);
var_dump($archive->addEmptyDir('test'));
$archive->close();
unlink('__test.zip');
?>
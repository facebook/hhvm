<?php

$dir = dirname(__FILE__);
$file = '__tmp14962.txt';
$fullpath = $dir . '/' . $file;
$za = new ZipArchive;
$za->open($dir . '/__14962.zip', ZIPARCHIVE::CREATE);
$za->addFromString($file, '1234');
$za->close();

if (!is_file($dir . "/__14962.zip")) {
	die('failed to create the archive');
}
$za = new ZipArchive;
$za->open($dir . '/__14962.zip');
$za->extractTo($dir, NULL);
$za->close();

if (is_file($fullpath)) {
	unlink($fullpath);
	echo "Ok";
}
unlink($dir . '/' . '__14962.zip');
?>
<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');
var_dump(is_file($tempfile));
var_dump(is_dir($tempfile));
var_dump(is_dir('/tmp'));
var_dump(is_link($tempfile));
var_dump(is_executable($tempfile));
chmod($tempfile, 0777);
clearstatcache();
var_dump(is_executable($tempfile));
var_dump(is_writable($tempfile));
var_dump(is_readable($tempfile));
var_dump(is_uploaded_file($tempfile));
var_dump(filetype($tempfile));

$streamfile = 'file://' . $tempfile;
var_dump(is_file($streamfile));
var_dump(is_dir($streamfile));

unlink($tempfile);

mkdir($tempfile);
var_dump(is_dir($tempfile));
rmdir($tempfile);
clearstatcache();
var_dump(is_dir($tempfile));
var_dump(is_writable($tempfile));
var_dump(is_readable($tempfile));
var_dump(is_executable($tempfile));

$tempfile = tempnam(getcwd(), 'vmextfiletest');
$relativetempfile = './' . basename($tempfile);
var_dump(is_file($relativetempfile));
var_dump(is_dir($relativetempfile));
var_dump(is_link($relativetempfile));
var_dump(is_executable($relativetempfile));
chmod($tempfile, 0777);
clearstatcache();
var_dump(is_executable($relativetempfile));
var_dump(is_writable($relativetempfile));
var_dump(is_readable($relativetempfile));
var_dump(is_uploaded_file($relativetempfile));
var_dump(filetype($relativetempfile));

unlink($tempfile);

mkdir($tempfile);
var_dump(is_dir($relativetempfile));
rmdir($tempfile);
var_dump(is_dir($relativetempfile));

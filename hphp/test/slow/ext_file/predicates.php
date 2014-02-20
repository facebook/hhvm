<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');
var_dump(is_file($tempfile));
var_dump(is_dir($tempfile));
var_dump(is_dir('/tmp'));
var_dump(is_link($tempfile));
var_dump(is_executable($tempfile));
chmod($tempfile, 0777);
var_dump(is_executable($tempfile));
var_dump(is_writable($tempfile));
var_dump(is_readable($tempfile));
var_dump(is_uploaded_file($tempfile));
var_dump(filetype($tempfile));

unlink($tempfile);

mkdir($tempfile);
var_dump(is_dir($tempfile));
rmdir($tempfile);
var_dump(is_dir($tempfile));

$tempfile = tempnam(getcwd(), 'vmextfiletest');
$relativetempfile = './' . basename($tempfile);
unlink($tempfile);
mkdir($tempfile);
var_dump(is_dir($relativetempfile));
rmdir($tempfile);
var_dump(is_dir($relativetempfile));

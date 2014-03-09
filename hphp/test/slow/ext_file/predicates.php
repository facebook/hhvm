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

unlink($tempfile);

mkdir($tempfile);
var_dump(is_dir($tempfile));
rmdir($tempfile);
clearstatcache();
var_dump(is_dir($tempfile));

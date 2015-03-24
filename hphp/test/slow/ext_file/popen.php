<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, 'testing popen');
fclose($f);

var_dump(file_exists($tempfile));
var_dump(file_exists('somethingthatdoesntexist'));

$f = popen("cat $tempfile", 'r');
var_dump(fread($f, 20));
pclose($f);

$old_dir_path = getcwd();
$filename = str_replace('/tmp/', '', $tempfile);
chdir('/tmp');
$f = popen("cat $filename", 'r');
var_dump(fread($f, 20));
pclose($f);
chdir($old_dir_path);

unlink($tempfile);

<?php

$magic_file = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'magic';

$ret = @finfo_open(FILEINFO_NONE, $magic_file . ".non-exits");
var_dump($ret);

$dir = __DIR__ . "/test-folder";
@mkdir($dir);

$magic_file_copy = $dir . "/magic.copy";
$magic_file_copy2 = $magic_file_copy . "2";
copy($magic_file, $magic_file_copy);
copy($magic_file, $magic_file_copy2);

$ret = finfo_open(FILEINFO_NONE, $dir);
var_dump($ret);

$ret = @finfo_open(FILEINFO_NONE, $dir);
var_dump($ret);

$ret = @finfo_open(FILEINFO_NONE, $dir. "/non-exits-dir");
var_dump($ret);

// write some test files to test folder
file_put_contents($dir . "/test1.txt", "string\n> Core\n> Me");
file_put_contents($dir . "/test2.txt", "a\nb\n");
@mkdir($dir . "/test-inner-folder");

finfo_open(FILEINFO_NONE, $dir);
echo "DONE: testing dir with files\n";

rmdir($dir . "/test-inner-folder");
unlink($dir . "/test1.txt");
unlink($dir . "/test2.txt");

unlink($magic_file_copy);
unlink($magic_file_copy2);
rmdir($dir);
?>
===DONE===
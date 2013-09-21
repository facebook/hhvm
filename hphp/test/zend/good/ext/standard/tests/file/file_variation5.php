<?php

$script_directory = dirname(__FILE__);
chdir($script_directory);
$test_dirname = basename(__FILE__, ".php") . "testdir";
mkdir($test_dirname);

$filepath = __FILE__ . ".tmp";
$filename = basename($filepath);
$fd = fopen($filepath, "w+");
fwrite($fd, "Line 1\nLine 2\nLine 3");
fclose($fd);

echo "file() on a path containing .. and .\n";
var_dump(file("./$test_dirname/../$filename"));

echo "\nfile() on a path containing .. with invalid directories\n";
var_dump(file("./$test_dirname/bad_dir/../../$filename"));
 
echo "\nfile() on a linked file\n";
$linkname = "somelink";
var_dump(symlink($filepath, $linkname));
var_dump(file($linkname));
var_dump(unlink($linkname));

echo "\nfile() on a relative path from a different working directory\n";
chdir($test_dirname);
var_dump(file("../$filename"));
chdir($script_directory);

chdir($script_directory);
rmdir($test_dirname);
unlink($filepath);

?>
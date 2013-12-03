<?php
chdir(__DIR__);
ini_set("open_basedir", ".");
require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("readlink", FALSE);

chdir($initdir);

$target = ($initdir."/test/bad/bad.txt");
$symlink = ($initdir."/test/ok/symlink.txt");
var_dump(symlink($target, $symlink));

chdir($initdir."/test/ok");

var_dump(readlink("symlink.txt"));
var_dump(readlink("../ok/symlink.txt"));
var_dump(readlink("../ok/./symlink.txt"));
var_dump(readlink("./symlink.txt"));
var_dump(readlink($initdir."/test/ok/symlink.txt"));

$target = ($initdir."/test/ok/ok.txt");
$symlink = ($initdir."/test/ok/symlink.txt");
var_dump(symlink($target, $symlink));
var_dump(readlink($symlink));

test_open_basedir_after("readlink");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
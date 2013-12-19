<?php
chdir(__DIR__);
ini_set("open_basedir", ".");
require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("linkinfo", FALSE);

chdir($initdir);

$target = ($initdir."/test/bad/bad.txt");
$symlink = ($initdir."/test/ok/symlink.txt");
var_dump(symlink($target, $symlink));

chdir($initdir."/test/ok");

var_dump(linkinfo("symlink.txt"));
var_dump(linkinfo("../ok/symlink.txt"));
var_dump(linkinfo("../ok/./symlink.txt"));
var_dump(linkinfo("./symlink.txt"));
var_dump(linkinfo($initdir."/test/ok/symlink.txt"));

$target = ($initdir."/test/ok/ok.txt");
$symlink = ($initdir."/test/ok/symlink.txt");
var_dump(symlink($target, $symlink));
var_dump(linkinfo($symlink));
var_dump(unlink($symlink));

test_open_basedir_after("linkinfo");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
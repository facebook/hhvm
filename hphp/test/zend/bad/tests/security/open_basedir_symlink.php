<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("symlink");

$target = ($initdir."/test/ok/ok.txt");
var_dump(symlink($target, "../bad/symlink.txt"));
var_dump(symlink($target, "../symlink.txt"));
var_dump(symlink($target, "../bad/./symlink.txt"));
var_dump(symlink($target, "./.././symlink.txt"));

$symlink = ($initdir."/test/ok/symlink.txt");
var_dump(symlink("../bad/bad.txt", $symlink));
var_dump(symlink("../bad", $symlink));
var_dump(symlink("../bad/./bad.txt", $symlink));
var_dump(symlink("../bad/bad.txt", $symlink));
var_dump(symlink("./.././bad", $symlink));

$target = ($initdir."/test/ok/ok.txt");

var_dump(symlink($target, $symlink));
var_dump(unlink($symlink));

var_dump(mkdir("ok2"));
$symlink = ($initdir."/test/ok/ok2/ok.txt");
var_dump(symlink("../ok.txt", $symlink)); // $target == (dirname($symlink)."/".$target) == ($initdir."/test/ok/ok.txt");
var_dump(unlink($symlink));

test_open_basedir_after("symlink");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("rename");

var_dump(rename("../bad/bad.txt", "rename.txt"));
var_dump(rename(".././bad/bad.txt", "rename.txt"));
var_dump(rename("../bad/../bad/bad.txt", "rename.txt"));
var_dump(rename("./.././bad/bad.txt", "rename.txt"));
var_dump(rename($initdir."/test/bad/bad.txt", "rename.txt"));

test_open_basedir_after("rename");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
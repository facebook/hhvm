<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("rmdir");

var_dump(rmdir("../bad"));
var_dump(rmdir(".././bad"));
var_dump(rmdir("../bad/../bad"));
var_dump(rmdir("./.././bad"));
var_dump(rmdir($initdir."/test/bad"));

test_open_basedir_after("rmdir");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
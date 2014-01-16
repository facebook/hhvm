<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("error_log");


var_dump(ini_set("error_log", $initdir."/test/bad/bad.txt"));
var_dump(ini_set("error_log", $initdir."/test/bad.txt"));
var_dump(ini_set("error_log", $initdir."/bad.txt"));
var_dump(ini_set("error_log", $initdir."/test/ok/ok.txt"));
var_dump(ini_set("error_log", $initdir."/test/ok/ok.txt"));

test_open_basedir_after("error_log");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
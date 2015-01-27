<?php
require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("is_executable");
test_open_basedir_error("is_executable");     

var_dump(is_executable("ok.txt"));
var_dump(is_executable("../ok/ok.txt"));
var_dump(is_executable($initdir."/test/ok/ok.txt"));
var_dump(is_executable($initdir."/test/ok/../ok/ok.txt"));

test_open_basedir_after("is_executable");
?>
<?php error_reporting(0); ?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
<?php
require_once "open_basedir.inc";
$initdir = getcwd();

test_open_basedir_before("dir");
test_open_basedir_error("dir");     

var_dump(dir($initdir."/test/ok/"));
var_dump(dir($initdir."/test/ok"));
var_dump(dir($initdir."/test/ok/../ok"));

test_open_basedir_after("dir");?>
<?php error_reporting(0); ?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
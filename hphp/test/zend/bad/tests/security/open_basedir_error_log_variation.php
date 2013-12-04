<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("error_log");

define("DESTINATION_IS_FILE", 3);

var_dump(error_log("Hello World!", DESTINATION_IS_FILE, $initdir."/test/bad/bad.txt"));
var_dump(error_log("Hello World!", DESTINATION_IS_FILE, $initdir."/test/bad.txt"));
var_dump(error_log("Hello World!", DESTINATION_IS_FILE, $initdir."/bad.txt"));
var_dump(error_log("Hello World!", DESTINATION_IS_FILE, $initdir."/test/ok/ok.txt"));

test_open_basedir_after("error_log");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
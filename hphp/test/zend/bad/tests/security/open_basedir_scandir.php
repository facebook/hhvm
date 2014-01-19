<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("scandir");
test_open_basedir_error("scandir");     

var_dump(scandir($initdir."/test/ok/"));
var_dump(scandir($initdir."/test/ok"));
var_dump(scandir($initdir."/test/ok/../ok"));

test_open_basedir_after("scandir");?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
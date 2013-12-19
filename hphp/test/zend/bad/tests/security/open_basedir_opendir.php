<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("opendir");
test_open_basedir_error("opendir");     

var_dump(opendir($initdir."/test/ok/"));
var_dump(opendir($initdir."/test/ok"));
var_dump(opendir($initdir."/test/ok/../ok"));

test_open_basedir_after("opendir");?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
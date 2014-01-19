<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("disk_free_space");
test_open_basedir_error("disk_free_space");     

var_dump(disk_free_space($initdir."/test/ok"));
test_open_basedir_after("disk_free_space");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
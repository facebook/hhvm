<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("mkdir");

var_dump(mkdir("../bad/blah"));
var_dump(mkdir("../blah"));
var_dump(mkdir("../bad/./blah"));
var_dump(mkdir("./.././blah"));

var_dump(mkdir($initdir."/test/ok/blah"));
var_dump(rmdir($initdir."/test/ok/blah"));
test_open_basedir_after("mkdir");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
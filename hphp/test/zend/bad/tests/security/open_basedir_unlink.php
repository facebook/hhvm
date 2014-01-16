<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("unlink");

var_dump(unlink("../bad/bad.txt"));
var_dump(unlink(".././bad/bad.txt"));
var_dump(unlink("../bad/../bad/bad.txt"));
var_dump(unlink("./.././bad/bad.txt"));
var_dump(unlink($initdir."/test/bad/bad.txt"));

test_open_basedir_after("unlink");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
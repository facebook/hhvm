<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("file");
test_open_basedir_error("file");
     
var_dump(file("ok.txt"));
var_dump(file("../ok/ok.txt"));
var_dump(file($initdir."/test/ok/ok.txt"));
var_dump(file($initdir."/test/ok/../ok/ok.txt"));

test_open_basedir_after("file");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
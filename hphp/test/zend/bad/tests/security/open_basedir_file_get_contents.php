<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("file_get_contents");
test_open_basedir_error("file_get_contents");
     
var_dump(file_get_contents("ok.txt"));
var_dump(file_get_contents("../ok/ok.txt"));
var_dump(file_get_contents($initdir."/test/ok/ok.txt"));
var_dump(file_get_contents($initdir."/test/ok/../ok/ok.txt"));

test_open_basedir_after("file_get_contents");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
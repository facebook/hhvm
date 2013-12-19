<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("file_put_contents");

var_dump(file_put_contents("../bad/bad.txt", "Hello World!"));
var_dump(file_put_contents(".././bad/bad.txt", "Hello World!"));
var_dump(file_put_contents("../bad/../bad/bad.txt", "Hello World!"));
var_dump(file_put_contents("./.././bad/bad.txt", "Hello World!"));
var_dump(file_put_contents($initdir."/test/bad/bad.txt", "Hello World!"));

test_open_basedir_after("file_put_contents");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("tempnam");

var_dump(tempnam("../bad", "test"));
var_dump(tempnam("..", "test"));
var_dump(tempnam("../", "test"));
var_dump(tempnam("/", "test"));
var_dump(tempnam("../bad/.", "test"));
var_dump(tempnam("./../.", "test"));
var_dump(tempnam("", "test"));

//absolute test
$file = tempnam($initdir."/test/ok", "test");
var_dump($file);
var_dump(unlink($file));

//relative test
$file = tempnam(".", "test");
var_dump($file);
var_dump(unlink($file));

$file = tempnam("../ok", "test");
var_dump($file);
var_dump(unlink($file));

test_open_basedir_after("tempnam");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
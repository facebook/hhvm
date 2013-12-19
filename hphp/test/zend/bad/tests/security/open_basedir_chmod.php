<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();

test_open_basedir_before("chmod");

var_dump(chmod("../bad", 0600));
var_dump(chmod("../bad/bad.txt", 0600));
var_dump(chmod("..", 0600));
var_dump(chmod("../", 0600));
var_dump(chmod("/", 0600));
var_dump(chmod("../bad/.", 0600));
var_dump(chmod("../bad/./bad.txt", 0600));
var_dump(chmod("./../.", 0600));

var_dump(chmod($initdir."/test/ok/ok.txt", 0600));
var_dump(chmod("./ok.txt", 0600));
var_dump(chmod("ok.txt", 0600));
var_dump(chmod("../ok/ok.txt", 0600));
var_dump(chmod("../ok/./ok.txt", 0600));
chmod($initdir."/test/ok/ok.txt", 0777);

test_open_basedir_after("chmod");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
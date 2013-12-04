<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("touch");

var_dump(touch("../bad"));
var_dump(touch("../bad/bad.txt"));
var_dump(touch(".."));
var_dump(touch("../"));
var_dump(touch("/"));
var_dump(touch("../bad/."));
var_dump(touch("../bad/./bad.txt"));
var_dump(touch("./../."));

var_dump(touch($initdir."/test/ok/ok.txt"));
var_dump(touch("./ok.txt"));
var_dump(touch("ok.txt"));
var_dump(touch("../ok/ok.txt"));
var_dump(touch("../ok/./ok.txt"));

test_open_basedir_after("touch");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
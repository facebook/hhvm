<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("fopen");

var_dump(fopen("../bad", "r"));
var_dump(fopen("../bad/bad.txt", "r"));
var_dump(fopen("..", "r"));
var_dump(fopen("../", "r"));
var_dump(fopen("/", "r"));
var_dump(fopen("../bad/.", "r"));
var_dump(fopen("../bad/./bad.txt", "r"));
var_dump(fopen("./../.", "r"));

var_dump(fopen($initdir."/test/ok/ok.txt", "r"));
var_dump(fopen("./ok.txt", "r"));
var_dump(fopen("ok.txt", "r"));
var_dump(fopen("../ok/ok.txt", "r"));
var_dump(fopen("../ok/./ok.txt", "r"));

test_open_basedir_after("fopen");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("link");

$target = ($initdir."/test/ok/ok.txt");
var_dump(link($target, "../bad/link.txt"));
var_dump(link($target, "../link.txt"));
var_dump(link($target, "../bad/./link.txt"));
var_dump(link($target, "./.././link.txt"));

$link = ($initdir."/test/ok/link.txt");
var_dump(link("../bad/bad.txt", $link));
var_dump(link("../bad", $link));
var_dump(link("../bad/./bad.txt", $link));
var_dump(link("../bad/bad.txt", $link));
var_dump(link("./.././bad", $link));

$target = ($initdir."/test/ok/ok.txt");

var_dump(link($target, $link));
var_dump(unlink($link));
test_open_basedir_after("link");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
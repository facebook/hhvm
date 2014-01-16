<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
test_open_basedir_before("copy");

var_dump(copy("ok.txt", "../bad"));
var_dump(copy("ok.txt", "../bad/bad.txt"));
var_dump(copy("ok.txt", ".."));
var_dump(copy("ok.txt", "../"));
var_dump(copy("ok.txt", "/"));
var_dump(copy("ok.txt", "../bad/."));
var_dump(copy("ok.txt", "../bad/./bad.txt"));
var_dump(copy("ok.txt", "./../."));

var_dump(copy("ok.txt", "copy.txt"));
var_dump(unlink("copy.txt"));
test_open_basedir_after("copy");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
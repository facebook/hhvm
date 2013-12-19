<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
test_open_basedir_before("parse_ini_file");
$directory = dirname(__FILE__);

var_dump(parse_ini_file("../bad"));
var_dump(parse_ini_file("../bad/bad.txt"));
var_dump(parse_ini_file(".."));
var_dump(parse_ini_file("../"));
var_dump(parse_ini_file("../bad/."));
var_dump(parse_ini_file("../bad/./bad.txt"));
var_dump(parse_ini_file("./../."));

test_open_basedir_after("parse_ini_file");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
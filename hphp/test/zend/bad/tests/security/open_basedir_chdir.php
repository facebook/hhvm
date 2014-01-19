<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
test_open_basedir_before("chdir");

var_dump(chdir("../bad"));
var_dump(chdir(".."));
var_dump(chdir("../"));
var_dump(chdir("/"));
var_dump(chdir("../bad/."));
var_dump(chdir("./../."));

test_open_basedir_after("chdir");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
<?php
require_once "open_basedir.inc";
test_open_basedir_before("copy");

var_dump(copy("../bad/bad.txt", "copy.txt"));
var_dump(unlink("copy.txt"));

test_open_basedir_after("copy");
?>
<?php error_reporting(0); ?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
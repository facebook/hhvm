<?php
require_once "open_basedir.inc";
test_open_basedir_array("stat");
?>
<?php error_reporting(0); ?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
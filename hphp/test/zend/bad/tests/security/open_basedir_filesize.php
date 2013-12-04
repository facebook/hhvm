<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
test_open_basedir("filesize");
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>
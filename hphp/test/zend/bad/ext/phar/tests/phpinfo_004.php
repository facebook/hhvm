<?php
parse_str("a=b", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

phpinfo(INFO_MODULES);
ini_set('phar.readonly',1);
ini_set('phar.require_hash',1);
phpinfo(INFO_MODULES);
?>
===DONE===
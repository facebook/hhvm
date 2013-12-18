<?php
parse_str("a=1&b=2&c=3&d=4&e=5", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

var_dump($_POST);
?>
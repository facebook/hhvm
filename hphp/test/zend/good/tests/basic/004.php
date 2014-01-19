<?php
parse_str("a=Hello+World&b=Hello+Again+World", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();
 
error_reporting(0);
echo "{$_POST['a']} {$_POST['b']}" ?>
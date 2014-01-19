<?php
parse_str("a[]=1&a[]]=3&a[[]=4", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

var_dump($_POST['a']); 
?>
<?php
parse_str("a[]=1&a[a]=1&a[b]=3", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

var_dump($_POST['a']); 
?>
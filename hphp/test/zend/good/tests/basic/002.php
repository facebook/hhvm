<?php
parse_str("a=Hello+World", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

echo $_POST['a']; ?>
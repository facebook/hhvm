<?php
parse_str("email=foo&password=bar&submit=Log+on", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

var_dump($_POST);
?>
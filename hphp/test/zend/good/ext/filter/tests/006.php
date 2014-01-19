<?php
parse_str("foo=<b>abc</b>", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();
 
echo filter_input(INPUT_POST, 'foo', FILTER_SANITIZE_STRIPPED);
?>
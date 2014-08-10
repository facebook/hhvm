<?php
$_COOKIE = http_parse_cookie("1=10;2=11;3=12;4=13;5=14");
_filter_snapshot_globals();

parse_str("1=0&2=1&3=2&4=3&5=4", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

parse_str("1=5&2=6&3=7&4=8&5=9", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();


import_request_variables("gpc", "_");
var_dump($_1, $_2, $_3, $_4, $_5);

echo "Done\n";
?>
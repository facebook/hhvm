<?php
$_COOKIE = http_parse_cookie("GET=10;POST=11;COOKIE=12;FILES=13;REQUEST=14");
_filter_snapshot_globals();

parse_str("GET=0&POST=1&COOKIE=2&FILES=3&REQUEST=4", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

parse_str("GET=5&POST=6&COOKIE=7&FILES=8&REQUEST=9", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();


import_request_variables("gpc", "_");
var_dump($_GET, $_POST, $_COOKIE, $_FILES, $_REQUEST);

echo "Done\n";
?>
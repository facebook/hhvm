<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip;
_filter_snapshot_globals();

parse_str("dummy=42", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();

ob_start("ob_gzhandler");
echo "hi\n";
?>
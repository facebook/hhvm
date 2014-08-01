<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip;
_filter_snapshot_globals();

parse_str("a=b", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

echo "hi\n";
?>
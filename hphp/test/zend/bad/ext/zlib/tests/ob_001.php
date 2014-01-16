<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip;

parse_str("a=b", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);

echo "hi\n";
?>
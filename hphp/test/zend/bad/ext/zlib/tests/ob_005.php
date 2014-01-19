<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip;

parse_str("dummy=42", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);

ob_start("ob_gzhandler");
ini_set("zlib.output_compression", 0);
echo "hi\n";
?>
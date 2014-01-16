<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip;

parse_str("dummy=42", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);

ini_set("zlib.output_compression", 1);
echo "hi\n";
?>
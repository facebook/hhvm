<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip;

ini_set("zlib.output_compression", 0);
echo "hi\n";
?>
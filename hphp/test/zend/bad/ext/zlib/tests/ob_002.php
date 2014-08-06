<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip;
_filter_snapshot_globals();

ini_set("zlib.output_compression", 0);
echo "hi\n";
?>

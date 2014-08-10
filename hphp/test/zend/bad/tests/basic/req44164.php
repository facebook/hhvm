<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip;
_filter_snapshot_globals();

header("Content-length: 200");
echo str_repeat("a", 200);

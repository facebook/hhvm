<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip;
_filter_snapshot_globals();

header('Vary: Cookie');
echo 'foo';
?>

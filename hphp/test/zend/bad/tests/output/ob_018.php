<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip,deflate;
_filter_snapshot_globals();

ob_start('ob_gzhandler');
?>

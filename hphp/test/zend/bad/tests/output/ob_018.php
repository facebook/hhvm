<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip,deflate;

ob_start('ob_gzhandler');
?>
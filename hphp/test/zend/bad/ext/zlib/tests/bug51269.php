<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip;

header('Vary: Cookie');
echo 'foo';
?>
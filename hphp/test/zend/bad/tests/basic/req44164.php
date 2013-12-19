<?php
$_ENV[HTTP_ACCEPT_ENCODING] = gzip;

header("Content-length: 200");
echo str_repeat("a", 200);
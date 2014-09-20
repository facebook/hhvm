<?php
$clientCtx = stream_context_create(['ssl' => [
    'cafile' => 'http://curl.haxx.se/ca/cacert.pem'
]]);
file_get_contents('https://github.com', false, $clientCtx);

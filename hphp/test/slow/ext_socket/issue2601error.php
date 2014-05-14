<?php
$flags = STREAM_CLIENT_CONNECT;
$uri = 'unix:///socket/not/found';

$resource = @stream_socket_client($uri, $errno, $errstr, 5, $flags);

if (!$resource) {
    echo "Error: $errstr ($errno)";
}

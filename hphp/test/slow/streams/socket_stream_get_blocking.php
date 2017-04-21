<?php

$stream = stream_socket_server('tcp://127.0.0.1:32000');
$meta = stream_get_meta_data($stream);
var_dump($meta['blocked']);

stream_set_blocking($stream, 1);
$meta = stream_get_meta_data($stream);
var_dump($meta['blocked']);

stream_set_blocking($stream, 0);
$meta = stream_get_meta_data($stream);
var_dump($meta['blocked']);

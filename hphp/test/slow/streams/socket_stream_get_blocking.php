<?php


<<__EntryPoint>>
function main_socket_stream_get_blocking() {
$stream = stream_socket_server('tcp://127.0.0.1:32000');
$meta = stream_get_meta_data($stream);
var_dump($meta['blocked']);

stream_set_blocking($stream, true);
$meta = stream_get_meta_data($stream);
var_dump($meta['blocked']);

stream_set_blocking($stream, false);
$meta = stream_get_meta_data($stream);
var_dump($meta['blocked']);
}

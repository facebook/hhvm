<?php

var_dump(socket_import_stream());
var_dump(socket_import_stream(1, 2));
var_dump(socket_import_stream(1));
var_dump(socket_import_stream(new stdclass));
var_dump(socket_import_stream(fopen(__FILE__, "rb")));
var_dump(socket_import_stream(socket_create(AF_INET, SOCK_DGRAM, SOL_UDP)));
$s = stream_socket_server("udp://127.0.0.1:58392", $errno, $errstr, STREAM_SERVER_BIND);
var_dump($s);
var_dump(fclose($s));
var_dump(socket_import_stream($s));


echo "Done.";
<?php
$port = rand(50000, 65535);

$tcp_socket = stream_socket_server('tcp://127.0.0.1:'.$port);

socket_set_timeout($tcp_socket, 0, 1000);

var_dump(fread($tcp_socket, 1));

fclose($tcp_socket);

?>
<?php

$tcp_socket = stream_socket_server('tcp://127.0.0.1:31337');
var_dump(socket_get_status($tcp_socket));
fclose($tcp_socket);

?>
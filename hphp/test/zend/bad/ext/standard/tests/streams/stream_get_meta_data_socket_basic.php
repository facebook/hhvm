<?php

$tcp_socket = stream_socket_server('tcp://127.0.0.1:31330');
var_dump(stream_get_meta_data($tcp_socket));
fclose($tcp_socket);

?>

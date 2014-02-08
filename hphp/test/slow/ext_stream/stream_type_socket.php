<?php
    $tcp_server = stream_socket_server("tcp://127.0.0.1:50000", $errno, $errstr);
    $udp_server = stream_socket_server("udp://127.0.0.1:50001", $errno, $errstr, STREAM_SERVER_BIND);
    $unix_server = stream_socket_server("unix:///tmp/stream_type_socket.unix.sock", $errno, $errstr, STREAM_SERVER_LISTEN);
    $udg_server = stream_socket_server("udg:///tmp/stream_type_socket.udg.sock", $errno, $errstr, STREAM_SERVER_LISTEN);

    var_dump(stream_get_meta_data($tcp_server)["stream_type"]);
    var_dump(stream_get_meta_data($udp_server)["stream_type"]);
    var_dump(stream_get_meta_data($unix_server)["stream_type"]);
    var_dump(stream_get_meta_data($udg_server)["stream_type"]);

    fclose($tcp_server);
    fclose($udp_server);
    fclose($unix_server);
    fclose($udg_server);
?>
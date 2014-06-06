<?php

function main() {
  $tryport = 50000;
  while ($tryport++ < 51000) {
    $tcp_server = @stream_socket_server("tcp://127.0.0.1:{$tryport}",
        $errno, $errstr);
    if ($tcp_server) break;
  }
  while ($tryport++ < 51000) {
    $udp_server = @stream_socket_server("udp://127.0.0.1:{$tryport}",
        $errno, $errstr, STREAM_SERVER_BIND);
    if ($udp_server) break;
  }

  // Unix socket tests disabled because HHVM does not currently
  // support Unix sockets.
  // $unix_server = stream_socket_server("unix:///tmp/stream_type_socket".
  //     ".unix.sock", $errno, $errstr, STREAM_SERVER_LISTEN);
  // $udg_server = stream_socket_server("udg:///tmp/stream_type_socket".
  //     ".udg.sock", $errno, $errstr, STREAM_SERVER_LISTEN);

  $socket_pair = stream_socket_pair(STREAM_PF_UNIX, STREAM_SOCK_STREAM,
                                    STREAM_IPPROTO_IP);

  // Note: PHP returns "tcp_socket/ssl" for this query,
  // even though the socket is clearly not an SSL socket.
  var_dump(stream_get_meta_data($tcp_server)["stream_type"]);

  var_dump(stream_get_meta_data($udp_server)["stream_type"]);

  // var_dump(stream_get_meta_data($unix_server)["stream_type"]);
  // var_dump(stream_get_meta_data($udg_server)["stream_type"]);

  var_dump(stream_get_meta_data($socket_pair[0])["stream_type"]);
  var_dump(stream_get_meta_data($socket_pair[1])["stream_type"]);

  fclose($tcp_server);
  fclose($udp_server);
  // fclose($unix_server);
  // fclose($udg_server);
  fclose($socket_pair[0]);
  fclose($socket_pair[1]);
}
main();

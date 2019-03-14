<?php

abstract final class GetRandomPortStatics {
  public static $base = -1;
}

function get_random_port() {
  if (GetRandomPortStatics::$base == -1) {
    GetRandomPortStatics::$base = 12345 + (int)((int)(microtime(false) * 100) % 30000);
  }
  return ++GetRandomPortStatics::$base;
}

function retry_bind_server() {
  for ($i = 0; $i < 20; ++$i) {
    $port = get_random_port();
    $address = "tcp://127.0.0.1:" . $port;

    $server = @stream_socket_server($address);
    if ($server !== false) {
      return array($port, $address, $server);
    }
  }
  throw new Exception("Couldn't bind server");
}


<<__EntryPoint>>
function main_tcp_transport_is_assumed() {
list($port, $_, $server) = retry_bind_server();

$stream = stream_socket_client('127.0.0.1:'.$port);
stream_socket_accept($server);
var_dump(stream_get_meta_data($stream)['wrapper_type']);
$stream = stream_socket_client('tcp://127.0.0.1:'.$port);
stream_socket_accept($server);
var_dump(stream_get_meta_data($stream)['wrapper_type']);
$stream = stream_socket_client('udp://127.0.0.1:'.$port);
var_dump(stream_get_meta_data($stream)['wrapper_type']);
}

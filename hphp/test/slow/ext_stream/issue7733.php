<?php
$sock_file = 'test.sock';
$socket = stream_socket_server("unix://".$sock_file, $errno, $errstr);
if (!$socket) {
  echo "$errstr ($errno)<br />\n";
} else {
  $socket_path = stream_socket_get_name($socket, false);
  echo "$socket_path\n";
  fclose($socket);
  unlink($sock_file);
}

$socket = stream_socket_server("tcp://0.0.0.0:80", $errno, $errstr);
if (!$socket) {
  echo "$errstr ($errno)<br />\n";
} else {
  $socket_path = stream_socket_get_name($socket, false);
  echo "$socket_path\n";
  fclose($socket);
}


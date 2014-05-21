<?php

$host = '127.0.0.1';
$socket = null;
while (!$socket) {
  $port = rand(1024, 65535);
  $socket = @stream_socket_server("tcp://$host:$port");
}
$pid = pcntl_fork();
$data_size = 100000000;
if ($pid) {
  // 3 tests
  for ($i = 0; $i < 3; $i++) {
    $s = stream_socket_accept($socket);
    $data = str_repeat('a', $data_size);
    fwrite($s, $data);
  }
  die();
}

function test($connect_timeout, $send_timeout) {
  global $host, $port, $data_size;
  $fd = fsockopen($host, $port, $err_msg, $err_no, $connect_timeout);
  if ($send_timeout > 0) {
    stream_set_timeout($fd, 0, (int)($send_timeout * 1000000));
  }

  return strlen(stream_get_contents($fd)) == $data_size;
}

var_dump(test(0.001, 0.5));
var_dump(test(0.5, 0.001));
var_dump(test(0.001, 0));

<?php

$socket = null;
while (!$socket) {
  $port = rand(50000, 65535);
  $socket = @stream_socket_server(
    "tcp://127.0.0.1:$port",
    $errno,
    $errstr,
    STREAM_SERVER_BIND|STREAM_SERVER_LISTEN
  );
}

sleep(1);
$time = microtime();
var_dump(stream_socket_client("tcp://127.0.0.1:$port", $errno, $errstr, 0));
$elapsed = microtime() - $time;
if ($elapsed > 1) {
  print "FAILURE";
} else {
  print "SUCCESS";
}

<?php

function error_handler($errno, $errstr) {
  # This warning is expected to happen once in a while in the loop below.
  if ($errstr !=
      'unable to bind to given adress [98]: Address already in use') {
    var_dump($errno, $errstr);
  }
}
set_error_handler('error_handler');

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

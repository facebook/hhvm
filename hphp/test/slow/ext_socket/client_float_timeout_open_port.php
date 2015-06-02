<?php
function random_free_port() {
  for ($i = 0; $i < 100; $i++) {
    $port = rand(50000, 65000);
    if ($socket = @socket_create_listen($port)) {
      socket_close($socket);
      return $port;
    }
  }
  return 0;
}

$port = random_free_port();
$time = microtime(true);
@stream_socket_client("tcp://172.0.0.1:$port", $errno, $errstr, 0.001);
$elapsed = microtime(true) - $time;
echo $errstr, "\n";
if ($elapsed < 1) {
  print "SUCCESS";
} else {
  print "FAILURE";
}

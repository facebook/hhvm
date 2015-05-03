<?php
$port = 62534;
$time = microtime(true);
@stream_socket_client("tcp://172.0.0.1:$port", $errno, $errstr, 0.001);
$elapsed = microtime(true) - $time;
echo $errstr, "\n";
if ($elapsed < 1) {
  print "SUCCESS";
} else {
  print "FAILURE";
}

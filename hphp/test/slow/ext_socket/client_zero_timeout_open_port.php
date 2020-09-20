<?hh


<<__EntryPoint>>
function main_client_zero_timeout_open_port() {
$socket = null;
while (!$socket) {
  $port = rand(50000, 65535);
  $errno = null;
  $errstr = null;
  $socket = @stream_socket_server(
    "tcp://127.0.0.1:$port",
    inout $errno,
    inout $errstr,
    STREAM_SERVER_BIND|STREAM_SERVER_LISTEN
  );
}

sleep(1);
$time = microtime();
$errno = null;
$errstr = null;
var_dump(stream_socket_client("tcp://127.0.0.1:$port", inout $errno, inout $errstr, 0.0));
$elapsed = microtime() - $time;
if ($elapsed > 1) {
  print "FAILURE";
} else {
  print "SUCCESS";
}
fclose($socket);
}

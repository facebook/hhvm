<?hh


<<__EntryPoint>>
function main_accept_block() :mixed{
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
$pid = pcntl_fork();
if ($pid) {
  $peername = null;
  var_dump(stream_socket_accept($socket, -1.0, inout $peername));
} else {
  $errno = null;
  $errstr = null;
  stream_socket_client("tcp://127.0.0.1:$port", inout $errno, inout $errstr, 2.0);
}
}

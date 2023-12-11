<?hh

abstract final class GetRandomPortStatics {
  public static $base = -1;
}

//////////////////////////////////////////////////////////////////////

// so we run on different range of ports every time
function get_random_port() :mixed{
  if (GetRandomPortStatics::$base == -1) {
    GetRandomPortStatics::$base = 12345 + (int)((int)(HH\Lib\Legacy_FIXME\cast_for_arithmetic(microtime(false)) * 100) % 30000);
  }
  return ++GetRandomPortStatics::$base;
}

function bind_random_port($socket, $address) :mixed{
  for ($i = 0; $i < 100; $i++) {
    $port = get_random_port();
    if (@socket_bind($socket, $address, $port)) return $port;
  }
  return 0;
}

function create_listen_random_port(inout $sock) :mixed{
  for ($i = 0; $i < 100; $i++) {
    $port = get_random_port();
    if ($sock = @socket_create_listen($port)) return $port;
  }
  return 0;
}

function pfsockopen_random_port(inout $fsock, $address) :mixed{
  $fsock = false;
  for ($i = 0; $i < 100; $i++) {
    $port = get_random_port();
    $errno = null;
    $errstr = null;
    $fsock = @pfsockopen($address, $port, inout $errno, inout $errstr);
    if ($fsock !== false) return $port;
  }
  return 0;
}

function get_client_server() :mixed{
  $server = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
  $port = bind_random_port($server, "127.0.0.1");
  var_dump($port != 0);
  var_dump(socket_listen($server));

  $client = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
  var_dump(socket_connect($client, "127.0.0.1", $port));

  $s = socket_accept($server);
  return vec[$client, $s];
}


///////////////////////////////////////////////////////////////////////////////

<<__EntryPoint>>
function main_ext_socket() :mixed{
$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
var_dump($s);

$s2 = false;
var_dump(create_listen_random_port(inout $s2) != 0);
var_dump($s2);

$fds = null;
var_dump(socket_create_pair(AF_UNIX, SOCK_STREAM, 0, inout $fds));
var_dump(count($fds));
var_dump($fds[0]);
var_dump($fds[1]);

var_dump(socket_get_option($s, SOL_SOCKET, SO_TYPE), SOCK_STREAM);

list($client, $s) = get_client_server();
var_dump($s);
var_dump(socket_write($client, "hello world"));
// this could fail with shorter returns, but it never does...
var_dump(socket_read($s, 100));

list($client, $s) = get_client_server();
$reads = vec[$s];
$ignore1 = $ignore2 = null;
var_dump(socket_select(inout $reads, inout $ignore1, inout $ignore2, 1, 0));
var_dump(socket_write($client, "next select will be 1"));
$reads = vec[$s];
var_dump(socket_select(inout $reads, inout $ignore1, inout $ignore2, 1, 0));

list($client, $s) = get_client_server();
$text = "send/recv";
var_dump(socket_send($client, $text, 4, 0));
$buffer = null;
var_dump(socket_recv($s, inout $buffer, 100, 0));
var_dump($buffer);

list($client, $s) = get_client_server();
$text = "more specific";
for ($i = 0; $i < 100; $i++) {
  $port = get_random_port();
  $res = socket_sendto($client, $text, 4, 0, "127.0.0.1", $port);
  if ($res !== false) break;
}
var_dump($res);
$name = null;
$vport = null;
var_dump(socket_recvfrom($s, inout $buffer, 100, 0, inout $name, inout $vport));
var_dump($buffer);

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$port = bind_random_port($s, "127.0.0.1");
var_dump($port != 0);
var_dump(socket_listen($s));
var_dump(socket_shutdown($s));

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$port = bind_random_port($s, "127.0.0.1");
var_dump($port != 0);
var_dump(socket_listen($s));
var_dump(socket_close($s));

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
@socket_bind($s, "127.0.0.1", 25);
if (socket_last_error($s) == 13) {
  var_dump(socket_strerror(13) == "Permission denied");
  socket_clear_error($s);
}
var_dump(socket_last_error($s));

$fsock = false;
$port = pfsockopen_random_port(inout $fsock, "udp://[0:0:0:0:0:0:0:1]");
var_dump($fsock);
var_dump($port != 0);
var_dump(fwrite($fsock, "foo") > 0);

$errnum = null;
$errstr = null;
$fsock2 = pfsockopen("udp://[::1]", $port, inout $errnum, inout $errstr);
var_dump($fsock2);
var_dump($fsock !== false);
var_dump($fsock != $fsock2);
var_dump($errnum);
var_dump($errstr);
}

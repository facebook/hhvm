<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

// so we run on different range of ports every time
function get_random_port() {
  static $base = -1;
  if ($base == -1) {
    $base = 12345 + (int)((int)(microtime(false) * 100) % 30000);
  }
  return ++$base;
}

function bind_random_port($socket, $address) {
  for ($i = 0; $i < 20; $i++) {
    $port = get_random_port();
    if (socket_bind($socket, $address, $port)) return $port;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

VERIFY(socket_create(AF_INET, SOCK_STREAM, SOL_TCP));

$port = get_random_port();
socket_create_listen($port);

VERIFY(socket_create_pair(AF_UNIX, SOCK_STREAM, 0, $fds));
VS(count($fds), 2);

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VS(socket_get_option($s, SOL_SOCKET, SO_TYPE), SOCK_STREAM);

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VERIFY(socket_connect($s, "facebook.com", 80));
VERIFY(socket_getpeername($s, $address));
VERIFY(!empty($address));

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VERIFY(socket_connect($s, "facebook.com", 80));
VERIFY(socket_getsockname($s, $address));
VERIFY(!empty($address));

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VERIFY(socket_set_block($s));

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VERIFY(socket_set_nonblock($s));

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VERIFY(socket_set_option($s, SOL_SOCKET, SO_RCVTIMEO,
                         array("sec" => 1, "usec" => 0)));

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VERIFY(socket_connect($s, "facebook.com", 80));

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$port = bind_random_port($s, "127.0.0.1");
VERIFY($port != 0);
VERIFY(socket_listen($s));

$server = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$port = bind_random_port($server, "127.0.0.1");
VERIFY($port != 0);
VERIFY(socket_listen($server));

$client = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VERIFY(socket_connect($client, "127.0.0.1", $port));

$s = socket_accept($server);
VERIFY(socket_write($client, "testing"));

// this could fail with shorter returns, but it never does...
VS(socket_read($s, 100), "testing");


$server = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$port = bind_random_port($server, "127.0.0.1");
VERIFY($port != 0);
VERIFY(socket_listen($server));

$client = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VERIFY(socket_connect($client, "127.0.0.1", $port));

$s = socket_accept($server);

$reads = array($s);
VS(socket_select($reads, $ignore1, $ignore2, 1, 0), 0);

VERIFY(socket_write($client, "testing"));
$reads = array($s);
VS(socket_select($reads, $ignore1, $ignore2, 1, 0), 1);

for ($i = 0; $i < 20; $i++) {
  $port = get_random_port();
  $server = socket_server("127.0.0.1", $port);
  if ($server !== false) break;
}
VERIFY($server !== false);

$client = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VERIFY(socket_connect($client, "127.0.0.1", $port));

$s = socket_accept($server);

$reads = array($s);
VS(socket_select($reads, $ignore1, $ignore2, 1, 0), 0);

VERIFY(socket_write($client, "testing"));
$reads = array($s);
VS(socket_select($reads, $ignore1, $ignore2, 1, 0), 1);

$server = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$port = bind_random_port($server, "127.0.0.1");
VERIFY($port != 0);
VERIFY(socket_listen($server));

$client = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VERIFY(socket_connect($client, "127.0.0.1", $port));

$s = socket_accept($server);
$text = "testing";
VERIFY(socket_send($client, $text, 4, 0));

VERIFY(socket_recv($s, $buffer, 100, 0));
VS($buffer, "test");

$server = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$port = bind_random_port($server, "127.0.0.1");
VERIFY($port != 0);
VERIFY(socket_listen($server));

$client = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
VERIFY(socket_connect($client, "127.0.0.1", $port));

$s = socket_accept($server);
$text = "testing";
VERIFY(socket_sendto($client, $text, 4, 0, "127.0.0.1", $port));

VERIFY(socket_recvfrom($s, $buffer, 100, 0, $name, $vport));
VS($buffer, "test");

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$port = bind_random_port($s, "127.0.0.1");
VERIFY($port != 0);
VERIFY(socket_listen($s));
VERIFY(socket_shutdown($s));

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$port = bind_random_port($s, "127.0.0.1");
VERIFY($port != 0);
VERIFY(socket_listen($s));
socket_close($s);

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
socket_bind($s, "127.0.0.1", 80);
if (socket_last_error($s) == 13) {
  VS(socket_strerror(13), "Permission denied");
  socket_clear_error($s);
}
VS(socket_last_error($s), 0);

<?php

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
  for ($i = 0; $i < 100; $i++) {
    $port = get_random_port();
    if (@socket_bind($socket, $address, $port)) return $port;
  }
  return 0;
}

function create_listen_random_port() {
  for ($i = 0; $i < 100; $i++) {
    $port = get_random_port();
    if (@socket_create_listen($port)) return $port;
  }
  return 0;
}

function get_client_server() {
  $server = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
  $port = bind_random_port($server, "127.0.0.1");
  var_dump($port != 0);
  var_dump(socket_listen($server));

  $client = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
  var_dump(socket_connect($client, "127.0.0.1", $port));

  $s = socket_accept($server);
  return array($client, $s);
}

///////////////////////////////////////////////////////////////////////////////

$s = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
var_dump($s);

var_dump(create_listen_random_port() != 0);

var_dump(socket_create_pair(AF_UNIX, SOCK_STREAM, 0, $fds));
var_dump(count($fds));

var_dump(socket_get_option($s, SOL_SOCKET, SO_TYPE), SOCK_STREAM);

list($client, $s) = get_client_server();
var_dump(socket_write($client, "hello world"));
// this could fail with shorter returns, but it never does...
var_dump(socket_read($s, 100));

list($client, $s) = get_client_server();
$reads = array($s);
var_dump(socket_select($reads, $ignore1, $ignore2, 1, 0));
var_dump(socket_write($client, "next select will be 1"));
$reads = array($s);
var_dump(socket_select($reads, $ignore1, $ignore2, 1, 0));

list($client, $s) = get_client_server();
$text = "send/recv";
var_dump(socket_send($client, $text, 4, 0));
var_dump(socket_recv($s, $buffer, 100, 0));
var_dump($buffer);

list($client, $s) = get_client_server();
$text = "more specific";
for ($i = 0; $i < 100; $i++) {
  $port = get_random_port();
  $res = socket_sendto($client, $text, 4, 0, "127.0.0.1", $port);
  if ($res !== false) break;
}
var_dump($res);
var_dump(socket_recvfrom($s, $buffer, 100, 0, $name, $vport));
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
@socket_bind($s, "127.0.0.1", 80);
if (socket_last_error($s) == 13) {
  var_dump(socket_strerror(13) == "Permission denied");
  socket_clear_error($s);
}
var_dump(socket_last_error($s));

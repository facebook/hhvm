<?hh

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

use namespace HH\Lib\{OS, _Private\_OS};

async function server(OS\FileDescriptor $server): Awaitable<void> {
  _OS\fcntl($server, _OS\F_SETFL, _OS\O_NONBLOCK);
  await _OS\poll_async($server, \STREAM_AWAIT_READ, 0);
  list($client, $client_addr)  = _OS\accept($server);
  print("Client address from accept():\n");
  \var_dump($client_addr);
  print("Client address from getpeername():\n");
  \var_dump(_OS\getpeername($client));

  _OS\write($client, "Hello, ");
  await _OS\poll_async($client, \STREAM_AWAIT_READ, 0);
  $data = _OS\read($client, 1024);
  printf("Server received: '%s'\n", $data);
}

async function client(_OS\sockaddr $addr): Awaitable<void> {
  $fd = _OS\socket($addr->sa_family, _OS\SOCK_STREAM, 0);
  _OS\fcntl($fd, _OS\F_SETFL, _OS\O_NONBLOCK);
  try {
    _OS\connect($fd, $addr);
  } catch (\Exception $e) {
    if ($e->getCode() !== _OS\EINPROGRESS) {
      throw $e;
    }
  }
  // connect(2) documents non-blocking sockets as being ready for write when
  // complete...
  await _OS\poll_async($fd, \STREAM_AWAIT_WRITE, 0);
  print("Client address from getsockname():\n");
  \var_dump(_OS\getsockname($fd));
  print("Server address from getpeername():\n");
  \var_dump(_OS\getpeername($fd));
  // ... but we want the server to send us data.
  await _OS\poll_async($fd, \STREAM_AWAIT_READ, 0);
  $data = _OS\read($fd, 1024);
  printf("Client received: '%s'\n", $data);
  _OS\write($fd, "world!");
}

async function do_test(_OS\sockaddr $sa): Awaitable<void> {
  print("Initial server address:\n");
  \var_dump($sa);
  $server = _OS\socket($sa->sa_family, _OS\SOCK_STREAM, 0);
  _OS\bind($server, $sa);
  _OS\listen($server, 128);

  // We bound to port 0, so we got a random one. To connect, we need to find
  // the real port number.
  $addr = _OS\getsockname($server);
  print("Server address from getsockname():\n");
  var_dump($addr);

  concurrent {
    await server($server);
    await client($addr);
  }
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  print("***** Testing AF_INET server ***\n");
  $sin = new _OS\sockaddr_in(0, _OS\INADDR_LOOPBACK);
  await do_test($sin);

  print("\n\n***** Testing AF_UNIX server ***\n");
  $path = sys_get_temp_dir().'/'.'sock';
  $sun = new _OS\sockaddr_un_pathname($path);
  try {
    await do_test($sun);
  } finally {
    unlink($path);
  }
}

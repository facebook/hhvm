<?php

// Though unlikely, this is potentially flaky, but this is what
// the ext/ext_socket.php test does
function get_random_port() {
  return rand(10000, 60000);
}

// Given a socket, binds it to a random port on localhost. Again, potentially
// flaky, but this is what the ext/ext_socket.php test doeds
function bind_random_port($socket) {
  while (true) {
    $port = get_random_port();
    if (@socket_bind($socket, 'localhost', $port)) return $port;
  }
}

// Starts a server on the given port running the given file.
function start_server($file, $port, $socket) {
  $hhvm = PHP_BINARY;
  $env =  $_ENV + ["XDEBUG_CONFIG" => "remote_port=$port"];
  $opts = explode(" ", "-c $file.ini $file");

  // Fork and start the server
  $pid = pcntl_fork();
  if ($pid === 0) {
    socket_close($socket);
    pcntl_exec($hhvm, $opts, $env);
  } else {
    echo "Started server process $pid\n";
    return $pid;
  }
}

// Starts a client. Returns the socket and port used
function start_client() {
  $socket = socket_create(AF_INET, SOCK_STREAM, 0);
  $port = bind_random_port($socket);
  assert(socket_listen($socket));
  echo "Client listening on port $port\n";
  return array($socket, $port);
}

// Reads a dbgp message from the socket
function read_msg($socket) {
  $bytes = 0;
  $message = "";
  do {
    $buffer = null;
    $result = socket_recv($socket, $buffer, 1024, 0);
    if (!$result) {
      trigger_error("Client socket error: " . socket_last_error(),
                     E_USER_ERROR);
    }
    $bytes += $result;
    $message .= $buffer;
  } while ($message[$bytes - 1] !== "\0");
  return $message;
}

function test($filename) {
  if (!file_exists($filename)) {
    trigger_error("File $filename does not exists.", E_USER_ERROR);
  } else if (!file_exists("$filename.ini")) {
    trigger_error("File $filename.ini must exist.", E_USER_ERROR);
  } else if (!file_exists("$filename.in")) {
    trigger_error("File $filename.in must exist.", E_USER_ERROR);
  }

  // Start the test on a remote server
  list($socket, $port) = start_client();
  $server = start_server($filename, $port, $socket);

  // Accept a connection
  echo "Accepting connections...\n";
  $fd = socket_accept($socket);
  socket_close($socket);
  echo "Connection established\n";

  // socket read, write loop
  $input = file("$filename.in");
  foreach ($input as $response) {
    $response = trim($response)."\0";
    echo read_msg($fd) . "\n";
    echo "(cmd) $response\n";
    socket_write($fd, $response);
  }

  // Close the socket then kill the server
  socket_close($fd);
  posix_kill($server, SIGTERM);

  // Wait for the server to die
  echo "No more commands. Waiting for server to die...\n";
  pcntl_waitpid($server, $status);
  echo "Server exited with status $status\n";
}

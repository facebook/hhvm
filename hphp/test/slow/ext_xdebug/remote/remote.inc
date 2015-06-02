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
  // Choose the config file
  $config = __DIR__."/test.ini";
  if (file_exists("$file.ini")) {
    $config = "$file.ini";
  }

  // Setup the command line opts
  $hhvm = PHP_BINARY;
  $env =  $_ENV + ["XDEBUG_CONFIG" => "remote_port=$port"];
  $opts = [__DIR__, $hhvm, "-c", $config, $file];

  // Fork and start the server
  $pid = pcntl_fork();
  if ($pid === 0) {
    socket_close($socket);
    pcntl_exec(__DIR__."/remote.sh", $opts, $env);
  } else {
    return $pid;
  }
}

// Starts a client. Returns the socket and port used
function start_client() {
  $socket = socket_create(AF_INET, SOCK_STREAM, 0);
  $port = bind_random_port($socket);
  assert(socket_listen($socket));
  return array($socket, $port);
}

// Formats the given message for output
function format_msg($message) {
  // Remove # of bytes + null characters
  $message = str_replace("\0", "", $message);
  $message = preg_replace("/^[0-9]+?(?=<)/", "", $message);

  // Remove strings that could change between runs
  $message = preg_replace('/appid="[0-9]+"/',
                          'appid=""', $message);
  $message = preg_replace('/engine version=".*?"/',
                          'engine version=""', $message);
  $message = preg_replace('/protocol_version=".*?"/',
                          'protocol_version=""', $message);
  $message = preg_replace('/ idekey=".*?"/', '', $message);
  $message = preg_replace('/address="[0-9]+"/',
                          'address=""', $message);

  return $message;
}

// Returns true if the given message is a stream
function is_stream($msg) {
  // This is hacky, but it works in all cases and doesn't require
  // parsing xml
  $prefix = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n<stream";
  return strpos($msg, $prefix) === 0;
}

// Reads a dbgp message from the socket
function read_msg($socket) {
  $bytes = 0;
  $message = "";
  do {
    $buffer = null;
    $result = socket_recv($socket, $buffer, 1024, 0);
    if (!$result) {
      trigger_error(
        "Client socket error: " . socket_strerror(socket_last_error($socket)),
        E_USER_ERROR
      );
    }
    $bytes += $result;
    $message .= $buffer;
  } while ($message[$bytes - 1] !== "\0");

  return format_msg($message);
}

function test($filename) {
  if (!file_exists($filename)) {
    trigger_error("File $filename does not exists.", E_USER_ERROR);
  } else if (!file_exists("$filename.in")) {
    trigger_error("File $filename.in must exist.", E_USER_ERROR);
  }

  // Start the test on a remote server
  list($socket, $port) = start_client();
  $server = start_server($filename, $port, $socket);

  // Accept a connection
  do {
    $fd = @socket_accept($socket);
  } while ($fd === false);
  socket_close($socket);

  // socket read, write loop
  $input = file("$filename.in");
  foreach ($input as $response) {
    $response = trim($response);
    if (count($response) === 0 || $response[0] === "#") {
      continue;
    }

    // We should always receive a single response, we may also receive
    // streams, which we do not want to respond to
    do {
      $msg = read_msg($fd);
      echo $msg . "\n";
    } while (is_stream($msg));

    echo "(cmd) $response\n";
    socket_write($fd, $response."\0");
  }

  // Close the socket then kill the server
  socket_close($fd);

  // NOTE: This will hang if the server is waiting for commands
  pcntl_waitpid($server, $status);
}

test(realpath($_SERVER['SCRIPT_FILENAME'] . '.test'));

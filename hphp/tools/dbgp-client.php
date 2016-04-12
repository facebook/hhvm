#!/usr/local/bin/php

<?php

/* Needed for pcntl_signal(). */
declare(ticks=1);

class Connection {
  public $fd = null;
  public $send_break = false;
  public $expect_responses = 1;
  public $port = 9000;
}

$conn = new Connection();

////////////////////////////////////////////////////////////////////////////////

function starts_with($big, $small) {
  $slen = strlen($small);
  return $slen === 0 || strncmp($big, $small, $slen) === 0;
}

/* SIGINT handler that sends a dbgp break command. */
function handle_sigint($sig) {
  assert($sig === SIGINT);

  global $conn;

  // No connection yet.
  if (!is_resource($conn->fd)) {
    exit(1);
  }

  // If we have a connection going, then send it a break command.
  $conn->send_break = true;
}

function format_socket_error($fd, $prefix) {
  $error = socket_last_error($fd);
  return $prefix . ": " . socket_strerror($error);
}

/* Starts a client.  Returns the socket and port used. */
function start_client($port) {
  $socket = socket_create(AF_INET, SOCK_STREAM, 0);
  @socket_set_option($socket, SOL_SOCKET, SO_REUSEADDR, 1);
  @socket_bind($socket, 'localhost', $port);
  $result = socket_listen($socket);
  assert($result);
  return array($socket, $port);
}

/* Formats the given dbgp response for output. */
function format_response($m) {
  // Remove # of bytes + null characters.
  $m = str_replace("\0", "", $m);
  $m = preg_replace("/^[0-9]+?(?=<)/", "", $m);

  // Remove strings that could change between runs.
  $m = preg_replace('/appid="[0-9]+"/', 'appid=""', $m);
  $m = preg_replace('/engine version=".*?"/', 'engine version=""', $m);
  $m = preg_replace('/protocol_version=".*?"/', 'protocol_version=""', $m);
  $m = preg_replace('/ idekey=".*?"/', '', $m);
  $m = preg_replace('/address="[0-9]+"/', 'address=""', $m);

  return $m;
}

/* Returns true iff the given message is a stream. */
function is_stream($msg) {
  // This is hacky, but it works in all cases and doesn't require parsing xml.
  $prefix = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n<stream";
  return starts_with($msg, $prefix);
}

/* Reads a dbgp message from the socket. */
function read_response($socket) {
  $bytes = 0;
  $message = "";

  do {
    $buffer = "";
    $result = @socket_recv($socket, $buffer, 1024, 0);
    if ($result === false) {
      return format_socket_error($socket, "Client socket error") . "\n";
    }
    $bytes += $result;
    $message .= $buffer;
  } while ($message !== "" && $message[$bytes - 1] !== "\0");

  return format_response($message);
}

/* Sends a command to the xdebug server.  Exits process on failure. */
function send_command($fd, $cmd) {
  $result = @socket_write($fd, "$cmd\0");
  if ($result === false) {
    $error = format_socket_error($fd, "Client socket error");
    echo "$error\n";
    exit(1);
  }
}

function parse_options() {
  $opts = getopt("p::h");

  global $conn;

  if (isset($opts["p"])) {
    $conn->port = $opts["p"];
  }

  if (isset($opts["h"])) {
    print_usage();
    exit(0);
  }
}

function print_usage() {
  echo "Usage:\n";
  echo "dbgp-client.php [-pPORT]\n";
}

function main() {
  assert(pcntl_signal(SIGINT, 'handle_sigint'));

  parse_options();

  echo "-- Simple DBGp Client --\n";

  global $conn;

  // Start the listening socket.
  list($socket, $port) = start_client($conn->port);

  echo "Listening on port $port\n";

  // Accept a connection.
  $fd = null;
  while (true) {
    $fd = @socket_accept($socket);

    if ($fd !== false) {
      echo "Connected to an XDebug server!\n";
      break;
    }
  }
  socket_close($socket);

  $conn->fd = $fd;

  while (true) {
    // Wait for the expected number of responses.  Normally we expect 1
    // response, but with the break command, we expect 2.
    $responses = "";
    while ($conn->expect_responses > 0) {
      $response = read_response($fd);

      if (starts_with($response, "Client socket error")) {
        break;
      }

      // Init packet doesn't end in </response>.
      $conn->expect_responses -= substr_count($response, "</response>");
      $conn->expect_responses -= substr_count($response, "</init>");
      $responses .= $response;
    }
    $conn->expect_responses = 1;

    // Might have been sent a Ctrl-c while waiting for the response.
    if ($conn->send_break) {
      send_command($fd, "break -i SIGINT\0");
      $conn->send_break = false;

      // We're expecting a response for the break command, and the command
      // before the break command.
      $conn->expect_responses = 2;
      continue;
    }

    // Echo back the response to the user if it isn't a stream.
    if (!is_stream($responses)) {
      echo "$responses\n";
    }

    // Received response saying we're stopping.
    if (strpos($responses, "status=\"stopped\"") > 0) {
      echo "-- Request ended, stopping --\n";
      break;
    }

    // Get a command from the user and send it.
    $line = readline("(dbgp) $ ");
    $line = trim($line);

    if ($line === "") {
      continue;
    }

    if (starts_with("quit", $line)) {
      echo "-- Quitting, request will continue running --\n";
      break;
    }

    send_command($fd, $line);
  }

  socket_close($fd);
  $conn->fd = null;
}

main();

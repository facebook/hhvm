<?php
$port = 0;
$server = null;
while (!$server) {
  $port = rand(50000, 65535);
  $server = @stream_socket_server(
    "tcp://127.0.0.1:$port",
    $errno,
    $errstr,
    STREAM_SERVER_BIND|STREAM_SERVER_LISTEN
  );
}

$pid = pcntl_fork();
if( $pid ) {
  test_server();
  pcntl_wait($status);
} else {
  test_client();
}

function read_all_data( $conn, $bytes ) {
  $all_data = '';
  $data = '';

  // Loop until we read all the bytes we expected or we hit an error.
  stream_set_timeout($conn, 1);
  while( $bytes > 0 && $data = fread($conn, $bytes) ) {
    $bytes -= strlen($data);
    $all_data .= $data;
  }

  return $bytes == 0 ? $all_data : false;
}

function test_server() {
  global $server;

  // The server only accepts once, but the client will call
  // stream_socket_client multiple times with the persistent flag.
  if( $conn = stream_socket_accept($server) ) {
    $requests_remaining = 3;
    while( $requests_remaining > 0 ) {
      $requests_remaining--;
      $data = read_all_data($conn, 4);
      if( $data === false ) {
        break;
      }

      echo "Server received request: $data\n";

      // Send response back to the client
      fwrite($conn, "pong", 4);
    }
  }

  fclose($server);
}

function test_client() {
  do_request();
  do_request();
  do_request();
}

function do_request() {
  global $port;

  $client = stream_socket_client(
    "tcp://127.0.0.1:$port",
    $errno,
    $errstr,
    1.0,
    STREAM_CLIENT_CONNECT|STREAM_CLIENT_PERSISTENT
  );

  if( $client === FALSE ) {
    echo "Failed to connect to server $errstr\n";
  }

  echo "Sending request to server...\n";
  if( fwrite($client, "ping", 4) == 0 ) {
    echo "Failed writing to socket.\n";
  }

  $data = read_all_data($client, 4);
  if( $data === false ) {
    return false;
  }

  echo "Client received response: $data\n";
  return true;
}

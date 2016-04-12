<?php

$pemfile = tempnam('/tmp', 'sslservertest');

$certdata = array('countryName' => 'US',
                  'stateOrProvinceName' => 'California',
                  'localityName' => 'Menlo Park',
                  'organizationName' => 'Test Corp',
                  'organizationalUnitName' => 'Test Team',
                  'commonName' => 'Foo Bar',
                  'emailAddress' => 'foo@bar.com');

// Generate the certificate
$pkey = openssl_pkey_new();
$cert = openssl_csr_new($certdata, $pkey);
$cert = openssl_csr_sign($cert, null, $pkey, 1);

// Generate and save the PEM file
$pem_passphrase = 'testing';
$pem = array();
openssl_x509_export($cert, $pem[0]);
openssl_pkey_export($pkey, $pem[1], $pem_passphrase);
if (file_put_contents($pemfile, implode($pem)) === false) {
  echo "Error writing PEM file.\n";
  die;
}

$context = stream_context_create();

stream_context_set_option($context, 'ssl', 'local_cert', $pemfile);
stream_context_set_option($context, 'ssl', 'passphrase', $pem_passphrase);
stream_context_set_option($context, 'ssl', 'allow_self_signed', true);
stream_context_set_option($context, 'ssl', 'verify_peer', false);

$schemes = array("tcp", "tls", "ssl");
$contexts = array(null, $context, $context);
$servers = array();
$ports = array();
foreach ($schemes as $i => $scheme) {
  $server = null;
  $port = 0;
  while (!$server) {
    $port = rand(50000, 65535);
    $server = @stream_socket_server(
      "$scheme://127.0.0.1:$port",
      $errno,
      $errstr,
      STREAM_SERVER_BIND|STREAM_SERVER_LISTEN,
      $contexts[$i]
    );
  }
  $servers[] = $server;
  $ports[] = $port;
}

foreach ($schemes as $i => $scheme) {
  $server = $servers[$i];
  $port = $ports[$i];

  $pid = pcntl_fork();
  if( $pid ) {
    echo "Testing $scheme://\n";
    test_server($server);
    pcntl_wait($status);
    exit;
  } else {
    test_client($scheme, $port);
  }
}

unlink($pemfile);


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

function test_server($server) {
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

function test_client($scheme, $port) {
  $clientcontext = null;
  if ($scheme != "tcp") {
    $clientcontext = stream_context_create();
    stream_context_set_option($clientcontext, 'ssl', 'allow_self_signed', true);
    stream_context_set_option($clientcontext, 'ssl', 'verify_peer', false);
  }

  do_request($scheme, $port, $clientcontext);
  do_request($scheme, $port, $clientcontext);
  do_request($scheme, $port, $clientcontext);
}

function do_request($scheme, $port, $context) {
  $client = stream_socket_client(
    "$scheme://127.0.0.1:$port",
    $errno,
    $errstr,
    1.0,
    STREAM_CLIENT_CONNECT|STREAM_CLIENT_PERSISTENT,
    $context
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

<?php

$file = '/etc/passwd'.chr(0).'asdf';

$opt_choices = array(
  array('ssl' => array('cafile' => $file)),
  array('ssl' => array('capath' => $file)),
  array('ssl' => array('local_cert' => $file)),
);

$socket = stream_socket_server('tcp://localhost:0', $errno, $errstr);
$name = stream_socket_get_name($socket, false);

$port = explode(":", $name)[1];

foreach ($opt_choices as $opts) {
  $ctx = stream_context_create($opts);
  $sock = stream_socket_client(
    sprintf('tls://localhost:%d', $port),
    $errno,
    $errstr,
    600,
    STREAM_CLIENT_CONNECT,
    $ctx
  );
  var_dump($sock);
}

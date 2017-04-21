<?php

$name = "";
$errstr = "Could not grab a random socket file in 10 tries.";
$errno = "";
$stream = null;

for ($i = 0; !$stream && $i < 10; $i++) {
  // Try building the stream several times.  It could fail if we are
  // unlucky and tempnam in another test uses the same name.
  $name =  tempnam(sys_get_temp_dir(), "socket");
  unlink($name);
  $socket = stream_socket_server("unix://".$name, $errno, $errstr);
}
if (!$socket) {
  echo "$errstr ($errno)\n";
} else {
  $socket_path = stream_socket_get_name($socket, false);
  var_dump($name == $socket_path);
  fclose($socket);
  unlink($name);
}

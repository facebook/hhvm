<?hh
<<__EntryPoint>> function main(): void {
for ($i=0; $i<100; $i++) {
  $port = rand(10000, 65000);
  /* Setup socket server */
  $errno = null;
  $errstr = null;
  $server = @stream_socket_server("udp://[::1]:$port", inout $errno, inout $errstr, STREAM_SERVER_BIND);
  if ($server) {
    break;
  }
}

  if (!$server) {
      exit('Unable to create AF_INET6 socket [server]');
  }

  /* Connect to it */
  $client = stream_socket_client("udp://[::1]:$port", inout $errno, inout $errstr);
  if (!$client) {
      exit('Unable to create AF_INET6 socket [client]');
  }

  fwrite($client, "ABCdef123\n");

  $data = fread($server, 10);
  var_dump($data);

  fclose($client);
  fclose($server);
}

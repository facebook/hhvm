<?hh
<<__EntryPoint>> function main(): void {
for ($i=0; $i<100; $i++) {
  $port = rand(10000, 65000);
  /* Setup socket server */
  $errno = null;
  $errstr = null;
  error_reporting(0);
  $server = stream_socket_server(
    "tcp://[::1]:$port",
    inout $errno,
    inout $errstr
  );
  error_reporting(E_ALL);
  if ($server) {
    break;
  }
}

  if (!$server) {
      exit('Unable to create AF_INET6 socket [server]');
  }

  /* Connect to it */
  $client = stream_socket_client("tcp://[::1]:$port", inout $errno, inout $errstr);
  if (!$client) {
      exit('Unable to create AF_INET6 socket [client]');
  }

  /* Accept that connection */
  $peername = null;
  $socket = stream_socket_accept($server, -1.0, inout $peername);
  if (!$socket) {
      exit('Unable to accept connection');
  }

  fwrite($client, "ABCdef123\n");

  $data = fread($socket, 10);
  var_dump($data);

  fclose($client);
  fclose($socket);
  fclose($server);
}

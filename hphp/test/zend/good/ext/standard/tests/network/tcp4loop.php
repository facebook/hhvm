<?hh // vim:ft=php:
<<__EntryPoint>>
function main_entry(): void {

    for ($i=0; $i<100; $i++) {
      $port = rand(10000, 65000);
      /* Setup socket server */
      $errno = null;
      $errstr = null;
      $server = @stream_socket_server(
        "tcp://127.0.0.1:$port",
        inout $errno,
        inout $errstr
      );
      if ($server) {
        break;
      }
    }
  	if (!$server) {
  		exit('Unable to create AF_INET socket [server]');
  	}

  	/* Connect to it */
  	$client = stream_socket_client("tcp://127.0.0.1:$port", inout $errno, inout $errstr);
  	if (!$client) {
  		exit('Unable to create AF_INET socket [client]');
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

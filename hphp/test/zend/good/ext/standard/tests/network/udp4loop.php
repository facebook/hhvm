<?hh

<<__EntryPoint>>
function entrypoint_udp4loop(): void {
      /* Setup socket server */
      for ($port = 31338; $port < 31500; ++$port) {
        $uri = "udp://127.0.0.1:$port";
        $errno = null;
        $errstr = null;
        $server = @stream_socket_server($uri, inout $errno, inout $errstr, STREAM_SERVER_BIND);
        if ($server) break;
      }
      if (!$server) {
          exit('Unable to create AF_INET socket [server]: ' . $errstr);
      }

      /* Connect to it */
      $client = stream_socket_client($uri, inout $errno, inout $errstr);
      if (!$client) {
          exit('Unable to create AF_INET socket [client]');
      }

      fwrite($client, "ABCdef123\n");

      $data = fread($server, 10);
      var_dump($data);

      fclose($client);
      fclose($server);
}

<?hh
<<__EntryPoint>> function main(): void {
    $server = socket_create(AF_INET6, SOCK_STREAM, getprotobyname('tcp'));
    if (!$server) {
        exit('Unable to create AF_INET6 socket [server]');
    }
    $bound = false;
    for($port = 31337; $port < 31357; ++$port) {
        // HACK: Stifle "port in use" warnings.
        if (@socket_bind($server, '::1', $port)) {
            $bound = true;
            break;
        }
    }
    if (!$bound) {
        exit("Unable to bind to [::1]:$port");
    }
    if (!socket_listen($server, 2)) {
        exit('Unable to listen on socket');
    }

    /* Connect to it */
    $client = socket_create(AF_INET6, SOCK_STREAM, getprotobyname('tcp'));
    if (!$client) {
        exit('Unable to create AF_INET6 socket [client]');
    }
    if (!socket_connect($client, '::1', $port)) {
        exit('Unable to connect to server socket');
    }

    /* Accept that connection */
    $socket = socket_accept($server);
    if (!$socket) {
        exit('Unable to accept connection');
    }

    socket_write($client, "ABCdef123\n");

    $data = socket_read($socket, 10, PHP_BINARY_READ);
    var_dump($data);

    socket_close($client);
    socket_close($socket);
    socket_close($server);
}

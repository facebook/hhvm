<?hh <<__EntryPoint>> function main(): void {
$sockdir = getenv('HPHP_TEST_SOCKETDIR') ?? sys_get_temp_dir();
$sock_path = sprintf("%s/%s.sock", $sockdir, uniqid());

if (file_exists($sock_path))
    exit('Temporary socket already exists.');

/* Setup socket server */
$server = socket_create(AF_UNIX, SOCK_STREAM, 0);
if (!$server) {
    exit('Unable to create AF_UNIX socket [server]');
}
if (!socket_bind($server,  $sock_path)) {
    exit("Unable to bind to $sock_path");
}
if (!socket_listen($server, 2)) {
    exit('Unable to listen on socket');
}

/* Connect to it */
$client = socket_create(AF_UNIX, SOCK_STREAM, 0);
if (!$client) {
    exit('Unable to create AF_UNIX socket [client]');
}
if (!socket_connect($client, $sock_path)) {
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
unlink($sock_path);
}

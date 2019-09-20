<?hh <<__EntryPoint>> function main(): void {
$uniqid = uniqid();
if (file_exists("/tmp/$uniqid.sock"))
    die('Temporary socket already exists.');

/* Setup socket server */
$errno = null;
$errstr = null;
$server = stream_socket_server(
  "unix:///tmp/$uniqid.sock",
  inout $errno,
  inout $errstr
);
if (!$server) {
    die('Unable to create AF_UNIX socket [server]');
}

/* Connect to it */
$client = stream_socket_client("unix:///tmp/$uniqid.sock", inout $errno, inout $errstr);
if (!$client) {
    die('Unable to create AF_UNIX socket [client]');
}

/* Accept that connection */
$peername = null;
$socket = stream_socket_accept($server, -1.0, inout $peername);
if (!$socket) {
    die('Unable to accept connection');
}

fwrite($client, "ABCdef123\n");

$data = fread($socket, 10);
var_dump($data);

fclose($client);
fclose($socket);
fclose($server);
unlink("/tmp/$uniqid.sock");
}

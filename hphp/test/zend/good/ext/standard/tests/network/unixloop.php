<?hh <<__EntryPoint>> function main(): void {
$sockdir = getenv('HPHP_TEST_SOCKETDIR') ?? sys_get_temp_dir();
$uniqid = uniqid();
if (file_exists($sockdir."/$uniqid.sock"))
    exit('Temporary socket already exists.');

/* Setup socket server */
$errno = null;
$errstr = null;
$server = stream_socket_server(
  "unix://".$sockdir."/$uniqid.sock",
  inout $errno,
  inout $errstr
);
if (!$server) {
    exit('Unable to create AF_UNIX socket [server]');
}

/* Connect to it */
$client = stream_socket_client("unix://".$sockdir."/$uniqid.sock", inout $errno, inout $errstr);
if (!$client) {
    exit('Unable to create AF_UNIX socket [client]');
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
unlink($sockdir."/$uniqid.sock");
}

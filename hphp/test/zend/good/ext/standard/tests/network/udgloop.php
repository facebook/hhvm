<?hh <<__EntryPoint>> function main(): void {
$sockdir = getenv('HPHP_TEST_SOCKETDIR') ?? sys_get_temp_dir();
$uniqid = uniqid();
if (file_exists($sockdir."/$uniqid.sock"))
    exit('Temporary socket '.$sockdir.'/$uniqid.sock already exists.');

/* Setup socket server */
$errno = null;
$errstr = null;
$server = stream_socket_server("udg://".$sockdir."/$uniqid.sock", inout $errno, inout $errstr, STREAM_SERVER_BIND);
if (!$server) {
    exit('Unable to create AF_UNIX socket [server]');
}

/* Connect to it */
$client = stream_socket_client("udg://".$sockdir."/$uniqid.sock", inout $errno, inout $errstr);
if (!$client) {
    exit('Unable to create AF_UNIX socket [client]');
}

fwrite($client, "ABCdef123\n");

$data = fread($server, 10);
var_dump($data);

fclose($client);
fclose($server);
unlink($sockdir."/$uniqid.sock");
}

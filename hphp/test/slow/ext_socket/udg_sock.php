<?hh


<<__EntryPoint>>
function main_udg_sock() {
$sockdir = getenv('HPHP_TEST_SOCKETDIR') ?? sys_get_temp_dir();
$socket = $sockdir.'/socktest'.rand();
$data = 'Data to be sent';

// Create the server side
$server = socket_create(AF_UNIX, SOCK_DGRAM, 0);
var_dump($server);
$ret = socket_set_nonblock($server);
var_dump($ret);
$ret = socket_bind($server, $socket , 0);
var_dump($ret);

$errno = null;
$errstr = null;
// Create the client and send/receive the data
$client = stream_socket_client("udg://$socket", inout $errno, inout $errstr);
var_dump($client);

fwrite($client, $data);

$readed = socket_read($server, 100);
var_dump($readed);

socket_close($server);
unlink($socket);
}

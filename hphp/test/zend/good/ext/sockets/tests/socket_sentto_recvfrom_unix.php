<?hh <<__EntryPoint>> function main(): void {
$socket = socket_create(AF_UNIX, SOCK_DGRAM, SOL_UDP); // cause warning
$socket = socket_create(AF_UNIX, SOCK_DGRAM, 0);
if (!$socket) {
    die('Unable to create AF_UNIX socket');
}
if (!socket_set_nonblock($socket)) {
    die('Unable to set nonblocking mode for socket');
}
$buf = null;
$from = null;
$port = null;
$sockdir = getenv('HPHP_TEST_SOCKETDIR') ?? sys_get_temp_dir();
var_dump(socket_recvfrom($socket, inout $buf, 12, 0, inout $from, inout $port)); //false (EAGAIN, no warning)
$address = sprintf("%s/%s.sock", $sockdir, uniqid());
if (!socket_bind($socket, $address)) {
    die("Unable to bind to $address");
}

$msg = "Ping!";
$len = strlen($msg);
try { $bytes_sent = socket_sendto($socket, $msg, $len, 0); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // cause warning
$bytes_sent = socket_sendto($socket, $msg, $len, 0, $address);
if ($bytes_sent == -1) {
    unlink($address);
    die('An error occurred while sending to the socket');
} else if ($bytes_sent != $len) {
    unlink($address);
    die($bytes_sent . ' bytes have been sent instead of the ' . $len . ' bytes expected');
}

$from = "";
$port = null;
var_dump(socket_recvfrom($socket, inout $buf, 0, 0, inout $from, inout $port)); // expect false
$bytes_received = socket_recvfrom($socket, inout $buf, 12, 0, inout $from, inout $port);
if ($bytes_received == -1) {
    unlink($address);
    die('An error occurred while receiving from the socket');
} else if ($bytes_received != $len) {
    unlink($address);
    die($bytes_received . ' bytes have been received instead of the ' . $len . ' bytes expected');
}
echo "Received $buf";

socket_close($socket);
unlink($address);
}

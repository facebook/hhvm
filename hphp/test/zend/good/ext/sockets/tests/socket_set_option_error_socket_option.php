<?hh <<__EntryPoint>> function main(): void {
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if (!$socket) {
        die('Unable to create AF_INET socket [socket]');
}

socket_set_option( $socket, SOL_SOCKET, 1, 1);
socket_close($socket);
error_reporting(0);
unlink(dirname(__FILE__) . '/006_root_check.tmp');
}

<?php
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

if (!$socket) {
        die('Unable to create AF_INET socket [socket]');
}
// wrong params
$retval_1 = socket_set_option( $socket, SOL_SOCKET, SO_BINDTODEVICE, "lo");
var_dump($retval_1);
$retval_2 = socket_set_option( $socket, SOL_SOCKET, SO_BINDTODEVICE, "ethIDONOTEXIST");
var_dump($retval_2);

socket_close($socket);
?>

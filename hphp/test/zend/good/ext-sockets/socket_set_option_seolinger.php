<?php
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

if (!$socket) {
        die('Unable to create AF_INET socket [socket]');
}
// wrong params
$retval_1 = socket_set_option( $socket, SOL_SOCKET, SO_LINGER, array());

// set/get comparison
$options = array("l_onoff" => 1, "l_linger" => 1);
$retval_2 = socket_set_option( $socket, SOL_SOCKET, SO_LINGER, $options);
$retval_3 = socket_get_option( $socket, SOL_SOCKET, SO_LINGER);

//l_linger not given
$options_2 = array("l_onoff" => 1);
var_dump(socket_set_option( $socket, SOL_SOCKET, SO_LINGER, $options_2));

var_dump($retval_2);
var_dump($retval_3["l_linger"] === $options["l_linger"]);
// value of l_onoff is not always 1, Darwin returns 128
var_dump((bool)$retval_3["l_onoff"] === (bool)$options["l_onoff"]);

socket_close($socket);
?>

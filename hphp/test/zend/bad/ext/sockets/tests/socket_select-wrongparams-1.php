<?php
$sockets = array();
$domain = AF_UNIX;
socket_create_pair($domain, SOCK_STREAM, 0, $sockets);

$write  = null;
$except = null;
$time   = -1;
var_dump(socket_select($sockets, $write, $except, $time));

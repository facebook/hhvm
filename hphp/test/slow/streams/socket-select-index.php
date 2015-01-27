<?php

$msg = "Hello";
$len = strlen($msg);

$sockets = array();
socket_create_pair(AF_UNIX, SOCK_STREAM, 0, $sockets);
socket_write($sockets[0], $msg, $len);

$fdset = array( 1 => $sockets[1] );
$write = $excep = array();
socket_select($fdset, $write, $excep, 0, 100);
print_r($fdset);

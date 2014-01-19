<?php
var_dump(socket_create_pair(AF_INET, null, null));

$domain = 'unknown';
var_dump(socket_create_pair($domain, SOCK_STREAM, 0, $sockets));

var_dump(socket_create_pair(AF_INET, null, null, $sockets));

var_dump(socket_create_pair(31337, null, null, $sockets));

var_dump(socket_create_pair(AF_INET, 31337, 0, $sockets));
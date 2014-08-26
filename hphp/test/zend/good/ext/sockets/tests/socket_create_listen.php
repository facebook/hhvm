<?php
$sock = socket_create_listen(rand(1025, 65535));
socket_getsockname($sock, $addr, $port); 
var_dump($addr, $port);

<?php
$sock = socket_create_listen(31338);
socket_getsockname($sock, $addr, $port); 
var_dump($addr, $port);
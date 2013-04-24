<?php
$sockets = stream_socket_pair(STREAM_PF_UNIX, STREAM_SOCK_STREAM, 0);

stream_set_timeout($sockets[1], 6000);

fwrite($sockets[0], b"foo");
var_dump(stream_get_contents($sockets[1], 3));

?>
<?php
$sockets = stream_socket_pair(STREAM_PF_UNIX, STREAM_SOCK_STREAM,
                              STREAM_IPPROTO_IP);
fclose($sockets[0]);
var_dump(feof($sockets[1]));

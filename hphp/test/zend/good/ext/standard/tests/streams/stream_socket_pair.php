<?php
$domain = (strtoupper(substr(PHP_OS, 0, 3) == 'WIN') ? STREAM_PF_INET : STREAM_PF_UNIX);
$sockets = stream_socket_pair($domain, STREAM_SOCK_STREAM, 0);
var_dump($sockets);
fwrite($sockets[0], b"foo");
var_dump(fread($sockets[1], strlen(b"foo")));
fclose($sockets[0]);
?>